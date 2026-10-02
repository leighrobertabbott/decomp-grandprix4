#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00428a53
void __cdecl FUN_00428a53(gp4::Regs* r) {
    uint8_t* car = (uint8_t*)r->esi;
    uint8_t* other = (uint8_t*)r->edi;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    int32_t limit = 8;
    if ((car[0x9c] & 0x10) != 0) {
        limit = 0x28;
    } else if ((car[0x179] & 0x10) != 0) {
        const int16_t delta = (int16_t)(uint16_t)(GP4_FIELD(uint16_t, car, 0x90)
                                                  - GP4_GLOBAL(uint16_t, 0x00874b66));
        int32_t magnitude = delta;
        if (magnitude < 0) magnitude = -magnitude;
        if (magnitude > 0x200) magnitude = 0x200;
        int32_t s = GP4_FIELD(int16_t, car, 0x9a);
        if (s > 0x2000) s = 0x2000;
        const int64_t product = (int64_t)magnitude * (int64_t)(0x2000 - s);
        edx = (uint32_t)((uint64_t)product >> 32);
        int32_t v = (int32_t)((uint32_t)((uint64_t)product >> 14) >> 1);
        if (v < 8) v = 8;
        if (v > 0xff) v = 0xff;
        eax = (uint32_t)v;
        limit = v;
    }

    const uint16_t diff = (uint16_t)(GP4_FIELD(uint16_t, car, 0x0e) - GP4_FIELD(uint16_t, car, 0x142));
    const uint16_t field_b4 = GP4_FIELD(uint16_t, car, 0xb4);
    const uint16_t opposed = (uint16_t)(field_b4 ^ diff);
    ebp = (ebp & 0xffff0000u) | opposed;
    if ((opposed & 0x8000u) != 0) {
        const uint16_t mag = (diff & 0x8000u) ? (uint16_t)(0u - diff) : diff;
        const uint16_t shifted = (uint16_t)(GP4_FIELD(int16_t, car, 0x9a) >> 7);
        const int16_t t = (int16_t)(uint16_t)((uint16_t)(mag >> 5) - shifted);
        eax = (uint32_t)(int32_t)t;
        edx = (edx & 0xffff0000u) | shifted;
        if (limit < (int32_t)t) limit = t;
    } else {
        eax = (eax & 0xffff0000u) | diff;
        edx = (edx & 0xffff0000u) | field_b4;
    }

    if ((car[0xfe] & 0x40) == 0 && (car[0x163] & 2) != 0) {
        const uint32_t gap = (uint32_t)(int32_t)GP4_FIELD(int16_t, car, 0x10a)
                             - GP4_FIELD(uint32_t, car, 0x1a0);
        eax = gap;
        if ((gap & 0x80000000u) == 0) {
            const bool flag71 = (car[0x71] & 8) != 0;
            uint32_t scaleAddr = 0;
            const uint8_t f51 = other[0x51];
            if ((f51 & 0x20) != 0) {
                if ((f51 & 8) != 0) {
                    if (!flag71) scaleAddr = 0x006223a8;
                } else {
                    if (flag71) scaleAddr = 0x006223a8;
                }
            } else {
                const uint16_t sum = (uint16_t)(GP4_FIELD(uint16_t, other, 0x14)
                                                + GP4_FIELD(uint16_t, other, 0x174));
                const uint16_t avg = (uint16_t)((int16_t)sum >> 1);
                eax = (eax & 0xffff0000u) | avg;
                const uint16_t avgMag = (avg & 0x8000u) ? (uint16_t)(0u - avg) : avg;
                edx = (edx & 0xffff0000u) | avgMag;
                if ((int16_t)avgMag < GP4_GLOBAL(int16_t, 0x006223b4)) {
                    const bool neg94 = GP4_FIELD(int16_t, car, 0x94) < 0;
                    scaleAddr = (flag71 == neg94) ? 0x006223ac : 0x006223b0;
                } else if ((avg & 0x8000u) != 0) {
                    if (!flag71) scaleAddr = 0x006223a8;
                } else {
                    if (flag71) scaleAddr = 0x006223a8;
                }
            }
            if (scaleAddr != 0) {
                const int32_t scale = GP4_GLOBAL(int32_t, scaleAddr);
                const int64_t product = (int64_t)(int32_t)eax * (int64_t)scale;
                edx = (uint32_t)((uint64_t)product >> 32);
                eax = (uint32_t)((uint64_t)product >> 14);
                if (limit < (int32_t)eax) limit = (int32_t)eax;
            }
        }
    }

    const int32_t cap = GP4_GLOBAL(int32_t, 0x006223a4);
    const uint32_t a = (uint32_t)limit;
    const uint32_t b = (uint32_t)cap;
    const uint32_t res = a - b;
    const uint32_t flags = (a < b ? 1u : 0u)
        | parityFlag(res)
        | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u)
        | (res & 0x80000000u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ res)) & 0x80000000u) ? 0x800u : 0u);
    if (limit > cap) limit = cap;
    GP4_FIELD(int16_t, car, 0x174) = (int16_t)limit;

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00428a53, FUN_00428a53, "regs(esi:ptr[0x1a8]:bytes, edi:ptr[0x17c]:bytes, eax:u32, edx:u32, ebp:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
