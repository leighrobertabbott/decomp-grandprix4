#include <gp4/gp4.h>
#include <gp4/flags.h>

// 22 slots of 0x1c bytes at 0x00b1b8d8; a slot is in use when its first dword is nonzero.
struct Slot_00b1b8d8 {
    uint32_t Used00;
    uint8_t Unknown04[0x12];
    uint8_t Flag16;
    uint8_t Unknown17[5];
};
static_assert(sizeof(Slot_00b1b8d8) == 0x1c, "slot size");

// FUNCTION: GP4 0x00541be7
// Builds a bit mask over the 22 slots: bit k is set when slot k is in use (first dword nonzero)
// and its byte at +0x16 is nonzero. ebx, ecx and edx are saved and restored (pushed twice);
// eax = mask, flags are those of the loop's last `or ecx, ecx` with ecx = 0 (ZF = PF = 1).
void __cdecl FUN_00541be7(gp4::Regs* r) {
    const Slot_00b1b8d8* slot = GP4_ARRAY(Slot_00b1b8d8, 0x00b1b8d8);
    uint32_t mask = 0u;
    uint32_t bit = 1u;
    for (int k = 0; k < 22; ++k) {
        if (slot->Used00 != 0u && slot->Flag16 != 0u) {
            mask |= bit;
        }
        bit <<= 1;
        slot += 1;
    }
    r->eax = mask;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x00541be7, FUN_00541be7, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_00541d5a+slotflag_00541be7")
