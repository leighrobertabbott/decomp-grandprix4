#include <gp4/gp4.h>

// Flags of an 8-bit `test`/logic result (CF = OF = AF = 0): parity of the low byte, ZF, SF.
static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t p = result & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return ((p & 1u) == 0u ? 0x04u : 0u)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// FUNCTION: GP4 0x00405e6e
// Enters car record esi into the track segment edi (forward): if the "entered" bit 2 of byte +0x8d
// is already set it returns at once (flags of that test); otherwise it sets the bit, clears bit 4 of
// +0x9c (set again when the segment's low two bits of +0x48 are clear), corrects the word +0x90 by
// the offset 0x00405e45 computes against the segment's link (+0x44), stores the segment's window
// words (+0x60 / +0x62, minus 0x100) at +0x11c / +0x11e, picks one of them for +0xb0 by the sign of
// +0x90, stores the segment at +0x1c and runs 0x004322a6, 0x00409416 and 0x004036ca. pushal/popal
// restore every register; the flags are those 0x004036ca leaves.
void __cdecl FUN_00405e6e(gp4::Regs* r) {
    uint8_t* car = (uint8_t*)r->esi;
    const uint32_t segment = r->edi;
    uint8_t* seg = (uint8_t*)segment;

    const uint32_t entered = GP4_FIELD(uint8_t, car, 0x8d) & 2u;
    if (entered != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(entered);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x00405f76, c);
    GP4_FIELD(uint8_t, car, 0x8d) = (uint8_t)(GP4_FIELD(uint8_t, car, 0x8d) | 2u);
    GP4_FIELD(uint8_t, car, 0x9c) = (uint8_t)(GP4_FIELD(uint8_t, car, 0x9c) & 0xefu);

    const uint32_t low = GP4_FIELD(uint8_t, seg, 0x48) & 3u;
    c.eax = (c.eax & 0xffffff00u) | low;
    if (low == 0u) {
        GP4_FIELD(uint8_t, car, 0x9c) = (uint8_t)(GP4_FIELD(uint8_t, car, 0x9c) | 0x10u);
    }

    c.edi = GP4_FIELD(uint32_t, seg, 0x44);
    if (c.edi != 0u) {
        gp4::call_regs(0x00405e45, c);
        GP4_FIELD(uint16_t, car, 0x90) = (uint16_t)(GP4_FIELD(uint16_t, car, 0x90) - (uint16_t)c.eax);
    }
    c.edi = segment;

    uint16_t ax = (uint16_t)(GP4_FIELD(uint16_t, seg, 0x60) - 0x100u);
    GP4_FIELD(uint16_t, car, 0x11c) = ax;
    ax = (uint16_t)(GP4_FIELD(uint16_t, seg, 0x62) - 0x100u);
    GP4_FIELD(uint16_t, car, 0x11e) = ax;
    if (GP4_FIELD(int16_t, car, 0x90) < 0) {
        ax = GP4_FIELD(uint16_t, car, 0x11c);
    }
    GP4_FIELD(uint16_t, car, 0xb0) = ax;
    c.eax = (c.eax & 0xffff0000u) | ax;

    GP4_FIELD(uint32_t, car, 0x1c) = segment;
    gp4::call_regs(0x004322a6, c);
    gp4::call_regs(0x00409416, c);
    gp4::call_regs(0x004036ca, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00405e6e, FUN_00405e6e, "regs(esi:ptr[0x420]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+div_00403bb6")
