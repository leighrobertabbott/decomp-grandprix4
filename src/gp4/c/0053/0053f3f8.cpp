#include <gp4/gp4.h>
#include <gp4/flags.h>

// 12 slots of 0x40 bytes at 0x007ab1a4 (the sentinel 0x007ab4a4 is the byte after the table); a slot
// is free when the bytes at +0x00 and +0x20 are both zero.
struct Slot_007ab1a4 {
    uint8_t Used00;
    uint8_t Unknown01[0x1f];
    uint8_t Used20;
    uint8_t Unknown21[0x1f];
};
static_assert(sizeof(Slot_007ab1a4) == 0x40, "slot size");

// FUNCTION: GP4 0x0053f3f8
// Finds the first free slot of the 12-slot table at 0x007ab1a4. eax = its index, or 12 when every
// slot is in use. ebx and edx are saved and restored. The final `or bl, bl; jne` can never see
// bl == 0 (ebx is 0x007ab1a4 + k * 0x40, low byte 0xa4/0xe4/0x24/0x64), so `eax = -1` is dead; it is
// kept because the original executes it. The flags are those of `or bl, bl` (bl = low byte of ebx).
void __cdecl FUN_0053f3f8(gp4::Regs* r) {
    const Slot_007ab1a4* slot = GP4_ARRAY(Slot_007ab1a4, 0x007ab1a4);
    uint32_t ebx = 0x007ab1a4u;
    uint32_t eax = 0u;
    uint32_t dl = 0u;

    do {
        if (slot->Used00 == 0u && slot->Used20 == 0u) {
            dl = 0xffu;
        } else {
            slot += 1;
            ebx += 0x40u;
            eax += 1u;
        }
    } while (ebx != 0x007ab4a4u && dl != 0xffu);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ebx));
    if ((ebx & 0xffu) == 0u) eax = 0xffffffffu;
    r->eax = eax;
}
GP4_IMPL(0x0053f3f8, FUN_0053f3f8, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
