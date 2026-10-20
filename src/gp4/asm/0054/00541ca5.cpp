#include <gp4/gp4.h>
#include <gp4/flags.h>

// 22 slots of 0x1c bytes at 0x00b1b8d8; a slot is in use when its first dword is nonzero and
// its byte at +0x14 holds the kind.
struct Slot_00b1b8d8 {
    uint32_t Used00;
    uint8_t Unknown04[0x10];
    uint8_t Kind14;
    uint8_t Unknown15[7];
};
static_assert(sizeof(Slot_00b1b8d8) == 0x1c, "slot size");

// FUNCTION: GP4 0x00541ca5
// Counts the in-use slots whose byte at +0x14 equals the low byte of eax (eax = count). ebx, ecx
// and edx are preserved; the flags are those of the final `or ecx, ecx` (ecx = 0).
void __cdecl FUN_00541ca5(gp4::Regs* r) {
    const uint8_t kind = (uint8_t)r->eax;
    const Slot_00b1b8d8* slot = GP4_ARRAY(Slot_00b1b8d8, 0x00b1b8d8);
    uint32_t count = 0;

    for (uint32_t remaining = 22; remaining != 0; --remaining, ++slot) {
        if (slot->Used00 != 0u && slot->Kind14 == kind) {
            count += 1u;
        }
    }

    r->eax = count;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x00541ca5, FUN_00541ca5, "regs(eax:u32[0..3]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a")
