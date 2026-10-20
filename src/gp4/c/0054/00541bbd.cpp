#include <gp4/gp4.h>
#include <gp4/flags.h>

// 22 slots of 0x1c bytes at 0x00b1b8d8; a slot is in use when its first dword is nonzero.
struct Slot_00b1b8d8 {
    uint32_t Used00;
    uint8_t Unknown04[0x18];
};
static_assert(sizeof(Slot_00b1b8d8) == 0x1c, "slot size");

// Builds a bit mask over the 22 slots: bit k of eax is set when slot k is in use. ebx, ecx and edx
// are saved and restored; the flags are those of the loop's last `dec ecx` (1 -> 0, CF kept from
// the preceding `add ebx, 0x1c`, which does not carry).
// FUNCTION: GP4 0x00541bbd
void __cdecl FUN_00541bbd(gp4::Regs* r) {
    const Slot_00b1b8d8* slot = GP4_ARRAY(Slot_00b1b8d8, 0x00b1b8d8);
    uint32_t mask = 0u;
    uint32_t bit = 1u;
    uint32_t flags = 0u;
    for (uint32_t k = 0u; k < 22u; ++k) {
        if (slot->Used00 != 0u) {
            mask |= bit;
        }
        bit <<= 1;
        flags = gp4::flags::dec32(22u - k, gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot)), 0x1cu));
        slot += 1;
    }
    r->eax = mask;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00541bbd, FUN_00541bbd, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_00541d5a")
