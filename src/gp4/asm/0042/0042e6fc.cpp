#include <gp4/gp4.h>

// A car record (array at 0x66fef0); only the fields used here are named.
struct Unknown_0042e6fc_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x9c - 0x20];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xe2 - 0x9d];
    int16_t UnknownE2;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((((left ^ right) & (left ^ result)) >> 31) << 11);
}

// Flags of the 16-bit `cmp ax, 0`: CF = OF = AF = 0.
static inline uint32_t cmpZeroFlags16(uint16_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0042e6fc
// Compares two cars (ebx and edx) against the subject car esi through 0x0042dd4b (CF = not comparable;
// eax = signed distance): CF = 1 means ebx has to be rejected. pushal / popal: every register is
// preserved; only CF is the result (stc = reject), the other flags are those of the last compare.
//   * 0x42dd4b fails for (esi, ebx): CF = 0, for (esi, edx): CF = 1;
//   * with d1 = |distance to ebx| and d2 = |distance to edx|: both beyond 0xbd -> CF = (d2 > d1);
//     d2 > 0xbd and d1 <= 0xbd -> CF = 1; d2 <= 0xbd and d1 > 0xbd -> CF = 0;
//     both within 0xbd -> CF = (the word difference [ebx+0xe2] - [edx+0xe2], or 0x0040dc8a's, is negative).
void __cdecl FUN_0042e6fc(gp4::Regs* r) {
    const Unknown_0042e6fc_Car* subject = reinterpret_cast<const Unknown_0042e6fc_Car*>(static_cast<uintptr_t>(r->esi));
    const uint32_t other = r->ebx;
    const uint32_t first = r->edx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = first;
    c.edi = subject->Segment1C;
    gp4::call_regs(0x0042dd4b, c);
    if ((c.eflags & 1u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u);
        return;
    }
    int32_t d1 = static_cast<int32_t>(c.eax);
    if (d1 < 0) {
        d1 = -d1;
    }
    c.eax = static_cast<uint32_t>(d1);
    c.ebp = static_cast<uint32_t>(d1);

    // xchg ecx, ebx: the second call looks at edx instead of ebx
    c.ecx = other;
    c.ebx = first;
    c.edi = subject->Segment1C;
    gp4::call_regs(0x0042dd4b, c);
    if ((c.eflags & 1u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d4u) | 1u;
        return;
    }
    int32_t d2 = static_cast<int32_t>(c.eax);
    if (d2 < 0) {
        d2 = -d2;
    }

    uint32_t flags;
    bool reject;
    if (d2 > 0xbd) {
        flags = cmpFlags(static_cast<uint32_t>(d1), 0xbdu);
        if (d1 > 0xbd) {
            flags = cmpFlags(static_cast<uint32_t>(d2), static_cast<uint32_t>(d1));
            reject = d2 > d1;
        } else {
            reject = true;
        }
    } else {
        flags = cmpFlags(static_cast<uint32_t>(d1), 0xbdu);
        if (d1 > 0xbd) {
            reject = false;
        } else {
            const Unknown_0042e6fc_Car* carA = reinterpret_cast<const Unknown_0042e6fc_Car*>(static_cast<uintptr_t>(first));
            const Unknown_0042e6fc_Car* carB = reinterpret_cast<const Unknown_0042e6fc_Car*>(static_cast<uintptr_t>(other));
            uint16_t difference;
            if ((carA->Flags9C & 2u) != 0u || (carB->Flags9C & 2u) != 0u) {
                gp4::Regs pair = *r;
                pair.st_in = 0;
                pair.st_out = 0;
                pair.eax = d2;
                pair.esi = first;
                pair.ebx = other;
                gp4::call_regs(0x0040dc8a, pair);
                difference = static_cast<uint16_t>(pair.eax);
            } else {
                difference = static_cast<uint16_t>(carB->UnknownE2 - carA->UnknownE2);
            }
            flags = cmpZeroFlags16(difference);
            reject = static_cast<int16_t>(difference) < 0;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | (reject ? 1u : 0u);
}
GP4_IMPL(0x0042e6fc, FUN_0042e6fc, "regs(edx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677+car_0042e6fc, ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677+car_0042e6fc, esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677+car_0042e6fc) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+narrow_0042e6fc")
