#include <gp4/gp4.h>
#include <gp4/flags.h>

// Removes the first non-negative byte from a 22-byte list at ebx, searching from the last entry
// (+0x15) down to +0. Found at index k: the entries above it slide down one place (the last entry
// stays as it was; cl ends as that last byte when anything moved) and CF = 0 (clc) with the flags
// of the final `cmp edi, 0x15`. None found (every byte has bit 7 set): CF = 1 (stc) and the flags
// are those of the failing `sub edi, 1` (0 -> -1). edi is saved and restored.
// FUNCTION: GP4 0x00522b2f
void __cdecl FUN_00522b2f(gp4::Regs* r) {
    uint8_t* const list = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    int32_t index = 0x15;
    while (static_cast<int8_t>(list[index]) < 0) {
        if (index == 0) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 1u) | gp4::flags::CF);
            return;
        }
        --index;
    }
    if (index != 0x15) {
        uint8_t moved = 0u;
        for (; index < 0x15; ++index) {
            moved = list[index + 1];
            list[index] = moved;
        }
        r->ecx = (r->ecx & 0xffffff00u) | moved;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x15u, 0x15u) & ~gp4::flags::CF);
}
GP4_IMPL(0x00522b2f, FUN_00522b2f, "regs(ebx:ptr[0x20]:u8{0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0x00,0x7f}, ecx:u32) -> (ecx, cf, pf, af, zf, sf, of)")
