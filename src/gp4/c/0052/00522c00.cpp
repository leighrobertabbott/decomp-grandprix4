#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the three 22-entry dword tables at 0x00675970, 0x006759c8 and 0x00675a20 to 0x20000000. ebx ends as
// the byte offset 0x58 and ecx as 0 (the `loop` counter); the flags are those of the last `add ebx, 4`.
// FUNCTION: GP4 0x00522c00
void __cdecl FUN_00522c00(gp4::Regs* r) {
    uint32_t* first = GP4_ARRAY(uint32_t, 0x00675970);
    uint32_t* second = GP4_ARRAY(uint32_t, 0x006759c8);
    uint32_t* third = GP4_ARRAY(uint32_t, 0x00675a20);
    uint32_t offset = 0u;
    uint32_t arith = 0u;
    for (uint32_t remaining = 0x16u; remaining != 0u; --remaining) {
        first[offset >> 2] = 0x20000000u;
        second[offset >> 2] = 0x20000000u;
        third[offset >> 2] = 0x20000000u;
        arith = gp4::flags::add32(offset, 4u);
        offset += 4u;
    }
    r->ebx = offset;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522c00, FUN_00522c00, "regs() -> (ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
