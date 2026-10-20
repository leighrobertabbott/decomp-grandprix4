#include <gp4/gp4.h>
#include <gp4/flags.h>

// 12 slots of 0x40 bytes at 0x007ab1a4; a slot is free when the bytes at +0x00 and +0x20 are both zero.
struct Slot_007ab1a4 {
    uint8_t Used00;
    uint8_t Unknown01[0x1f];
    uint8_t Used20;
    uint8_t Unknown21[0x1f];
};
static_assert(sizeof(Slot_007ab1a4) == 0x40, "slot size");

// pushal/popal wrapper that builds the flag word for the item chain walk 0x0053e4db (head 0x00848042) from
// the current slot index byte [0x007ab4a4]: 0x19 when that slot is free (bytes +0x00 and +0x20 zero), else 4;
// plus 0x100 for index 0xb and 0x80 for index 0. popal restores every register, so only the flags
// 0x0053e4db leaves survive.
// FUNCTION: GP4 0x0053f3a7
void __cdecl FUN_0053f3a7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t bits = 0u;
    const uint32_t index = GP4_GLOBAL(uint8_t, 0x007ab4a4);
    const Slot_007ab1a4* slot = GP4_ARRAY(Slot_007ab1a4, 0x007ab1a4) + index;
    c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
    if (slot->Used00 != 0u) {
        bits |= 4u;
    } else if (slot->Used20 != 0u) {
        bits |= 4u;
    } else {
        bits |= 0x19u;
    }
    if (GP4_GLOBAL(uint8_t, 0x007ab4a4) == 0x0bu) {
        bits |= 0x100u;
    }
    if (GP4_GLOBAL(uint8_t, 0x007ab4a4) == 0u) {
        bits |= 0x80u;
    }
    c.eax = bits;
    c.esi = 0x00848042u;
    gp4::call_regs(0x0053e4db, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053f3a7, FUN_0053f3a7, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=slots_007ab1a4")
