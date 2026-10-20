#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic op (and/or) whose result is `result` (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Low 32 bits of the signed 64-bit product a*b shifted right by n (imul; shrd eax, edx, n).
static inline uint32_t mulShift(int32_t a, int32_t b, int n, uint32_t* high) {
    const int64_t product = (int64_t)a * (int64_t)b;
    *high = (uint32_t)((uint64_t)product >> 32);
    return (uint32_t)((uint64_t)product >> n);
}

// FUNCTION: GP4 0x00429431
// pushal/popal wrapper for the car record in esi, working on the segment record DAT_00601b24 and the
// fraction DAT_00601b20: DAT_00601b38 = 0x004035f7; unless bit 6 of byte +0x1ce is set,
// DAT_00601b3c = 0x0040362e (or the average with 0x004035c3 when bit 5 is set, which also clears
// word +0x296 and bit 5), then word +0x296 accumulates a 0x004293e4-scaled step clamped to
// +-DAT_00601af8 and DAT_00601afc = |word * DAT_00603854 >> 16| bounds the difference used as ecx.
// Then byte +0x1cf = mode (0x00403a1e), dword +0x290 = 0x00403ab9 of 0x00403938(DAT_00601b28) with
// DAT_00601b34 = DAT_00601b20 meanwhile, bit 7 of +0x1ce = side (edx == -1), bits 6/5 cleared.
// Registers are restored; the flags are those of the final `and byte [esi+0x1ce], 0xdf`.
void __cdecl FUN_00429431(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    s.edi = GP4_GLOBAL(uint32_t, 0x00601b24);
    s.eax = GP4_GLOBAL(uint32_t, 0x00601b20);
    gp4::call_regs(0x004035f7, s);
    s.ecx = s.eax;
    GP4_GLOBAL(uint32_t, 0x00601b38) = s.eax;

    if ((GP4_FIELD(uint8_t, car, 0x1ce) & 0x40u) == 0u) {
        s.edi = GP4_GLOBAL(uint32_t, 0x00601b24);
        s.eax = GP4_GLOBAL(uint32_t, 0x00601b20);
        gp4::call_regs(0x0040362e, s);
        GP4_GLOBAL(uint32_t, 0x00601b3c) = s.eax;

        if ((GP4_FIELD(uint8_t, car, 0x1ce) & 0x20u) != 0u) {
            s.edi = GP4_GLOBAL(uint32_t, 0x00601b24);
            s.eax = GP4_GLOBAL(uint32_t, 0x00601b20);
            gp4::call_regs(0x004035c3, s);
            GP4_GLOBAL(uint32_t, 0x00601b40) = s.eax;
            s.eax = (uint32_t)((int32_t)(GP4_GLOBAL(uint32_t, 0x00601b40) + GP4_GLOBAL(uint32_t, 0x00601b38)) >> 1);
            GP4_GLOBAL(uint32_t, 0x00601b3c) = s.eax;
            GP4_FIELD(uint16_t, car, 0x296) = 0;
            GP4_FIELD(uint8_t, car, 0x1ce) &= 0xdfu;
        }

        gp4::call_regs(0x004293e4, s);
        const uint32_t gain = s.eax;
        GP4_GLOBAL(uint32_t, 0x00601b94) = gain;

        const int32_t scale = GP4_GLOBAL(int32_t, 0x00603854);
        uint32_t high;
        const uint32_t step0 = mulShift((int32_t)gain, GP4_GLOBAL(int32_t, 0x00601af0), 14, &high);
        const uint32_t step = mulShift((int32_t)step0, scale, 16, &high);
        s.ebp = step;
        const uint32_t limit = mulShift((int32_t)gain, GP4_GLOBAL(int32_t, 0x00601af4), 14, &high);
        GP4_GLOBAL(uint32_t, 0x00601af8) = limit;

        uint32_t value = (uint32_t)(int32_t)GP4_FIELD(int16_t, car, 0x296) + step;
        if ((int32_t)value < 0) {
            uint32_t magnitude = 0u - value;
            if (!(magnitude < limit)) {
                magnitude = limit;
            }
            value = 0u - magnitude;
        } else if (!(value < limit)) {
            value = limit;
        }
        GP4_FIELD(uint16_t, car, 0x296) = (uint16_t)value;

        uint32_t band = mulShift((int32_t)value, scale, 16, &high);
        if ((int32_t)band < 0) {
            band = 0u - band;
        }
        GP4_GLOBAL(uint32_t, 0x00601afc) = band;
        s.eax = band;
        s.edx = high;
        s.ecx = GP4_GLOBAL(uint32_t, 0x00601b38);

        if ((GP4_FIELD(uint8_t, car, 0x26b) & 0x80u) == 0u) {
            const uint32_t centre = GP4_GLOBAL(uint32_t, 0x00601b3c);
            uint32_t diff = s.ecx - centre;
            uint32_t distance = diff;
            if ((int32_t)distance < 0) {
                distance = 0u - distance;
            }
            const uint32_t bound = GP4_GLOBAL(uint32_t, 0x00601afc);
            if ((int32_t)distance <= (int32_t)bound) {
                GP4_FIELD(uint16_t, car, 0x296) = 0;
            } else {
                distance = bound;
                if ((int32_t)diff < 0) {
                    distance = 0u - distance;
                }
                diff = distance;
            }
            s.edx = distance;
            s.ecx = diff + GP4_GLOBAL(uint32_t, 0x00601b3c);
        }
    }

    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(uint32_t, 0x00601b34) = GP4_GLOBAL(uint32_t, 0x00601b20);

    s.edi = GP4_GLOBAL(uint32_t, 0x00601b24);
    s.eax = s.edi;
    gp4::call_regs(0x00403a1e, s);
    GP4_FIELD(uint8_t, car, 0x1cf) = (uint8_t)s.eax;

    s.eax = GP4_GLOBAL(uint32_t, 0x00601b28);
    s.edx = GP4_FIELD(uint8_t, car, 0x1cf);
    gp4::call_regs(0x00403938, s);
    gp4::call_regs(0x00403ab9, s);
    GP4_FIELD(uint32_t, car, 0x290) = s.eax;

    if (s.edx == 0xffffffffu) {
        GP4_FIELD(uint8_t, car, 0x1ce) |= 0x80u;
    } else {
        GP4_FIELD(uint8_t, car, 0x1ce) &= 0x7fu;
    }
    GP4_FIELD(uint8_t, car, 0x1ce) &= 0xbfu;
    GP4_FIELD(uint8_t, car, 0x1ce) &= 0xdfu;
    const uint32_t flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x1ce));

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00429431, FUN_00429431, "regs(esi:ptr[0x2a0]:layout=car_004035c3+car_00431ffb+car_004035f7+car_0040362e) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+seg_00429431")
