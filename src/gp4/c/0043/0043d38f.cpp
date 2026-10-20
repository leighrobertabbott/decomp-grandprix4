#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043d38f
// edi = 0x160-byte track-segment record. Two vector lengths (0x0043d27c -> word 0x006077d8,
// 0x0043d2c4 -> word 0x006077da) are negated/scaled by 8, then two running counters
// (dwords 0x006269ac / 0x006269b0) either restart at 0 or gain a step that falls from 0x4000 as
// |word +0x14| grows past 0x140 (clamped at 0x200 beyond it). The counters become two 7-bit
// intensities written to bytes +0x10 (the larger one gets bit 7 when the first counter is not
// lower) and +0x50. Word +0x16 -/+ 0x340 against the two lengths gives the mask in dl; when it is
// not zero 0x0043d30c walks the ring marking the records with it.
// ebp ends as the step, ecx keeps its upper half and holds +0x16 + 0x340 in cx, eax is the 16-bit
// +0x16 - 0x340, edx is the mask, ebx is only changed by 0x0043d30c; the flags are those of
// "or dl, dl" (mask zero) or of 0x0043d30c.
void __cdecl FUN_0043d38f(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    gp4::call_regs(0x0043d27c, *r);
    gp4::call_regs(0x0043d2c4, *r);

    int16_t& lengthA = GP4_GLOBAL(int16_t, 0x006077d8);
    int16_t& lengthB = GP4_GLOBAL(int16_t, 0x006077da);
    lengthA = static_cast<int16_t>(lengthA * -8);
    lengthB = static_cast<int16_t>(lengthB * 8);

    int32_t slope = GP4_FIELD(int16_t, rec, 0x14);
    if (slope < 0) {
        slope = -slope;
    }
    slope -= 0x140;
    if (slope < 0) {
        slope = 0;
    }
    if (slope > 0x200) {
        slope = 0x200;
    }
    const uint32_t step = static_cast<uint32_t>(slope * -0x20) + 0x4000u;

    const int16_t lane = GP4_FIELD(int16_t, rec, 0x16);
    const int16_t low = static_cast<int16_t>(lane - 0x340);
    if (low < lengthA) {
        GP4_GLOBAL(uint32_t, 0x006269ac) = 0;
    } else {
        GP4_GLOBAL(uint32_t, 0x006269ac) += step;
    }
    const int16_t high = static_cast<int16_t>(lane + 0x340);
    if (high > lengthB) {
        GP4_GLOBAL(uint32_t, 0x006269b0) = 0;
    } else {
        GP4_GLOBAL(uint32_t, 0x006269b0) += step;
    }

    const uint32_t first = (GP4_GLOBAL(uint32_t, 0x006269ac) + 0x2000u) >> 14;
    const uint32_t second = (GP4_GLOBAL(uint32_t, 0x006269b0) + 0x2000u) >> 14;
    uint32_t outA;
    uint32_t outB;
    if (static_cast<int32_t>(first) < static_cast<int32_t>(second)) {
        outA = second > 0x7fu ? 0x7fu : second;
        outB = first > 0x7fu ? 0x7fu : first;
    } else {
        outA = (first > 0x7fu ? 0x7fu : first) | 0x80u;
        outB = second > 0x7fu ? 0x7fu : second;
    }
    GP4_FIELD(uint8_t, rec, 0x10) = static_cast<uint8_t>(outA);
    GP4_FIELD(uint8_t, rec, 0x50) = static_cast<uint8_t>(outB);

    uint32_t mask = 0;
    if (!(high < lengthB)) {
        mask |= 0x40u;
    }
    if (!(low > lengthA)) {
        mask |= 0x80u;
    }

    r->eax = static_cast<uint16_t>(low);
    r->edx = mask;
    r->ecx = (r->ecx & 0xffff0000u) | static_cast<uint16_t>(high);
    r->ebp = step;
    if (mask != 0u) {
        gp4::call_regs(0x0043d30c, *r);
    } else {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
    }
}
GP4_IMPL(0x0043d38f, FUN_0043d38f, "regs(edi:rec[0x8943a0,0x160,0..25], ebx:u32, ecx:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043d30c+words_008943a0+tab_0043d5d8+rec_0043d38f")
