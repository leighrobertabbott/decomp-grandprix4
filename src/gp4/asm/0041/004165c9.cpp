#include <gp4/gp4.h>

struct Unknown_004165c9 {
    uint8_t Unknown00[0xe];
    int16_t Unknown0E;
    uint8_t Unknown10[0x6f - 0x10];
    uint8_t Flags6F;
    uint8_t Unknown70[0xb4 - 0x70];
    int16_t UnknownB4;
    uint8_t UnknownB6[0xee - 0xb6];
    int16_t UnknownEE;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "test word, 0xffff": CF = OF = AF = 0.
static inline uint32_t testFlags16(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of "neg word": 0 - value.
static inline uint32_t negFlags16(uint32_t value) {
    const uint32_t result = (0u - value) & 0xffffu;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (value == 0x8000u ? 0x800u : 0u);
}

static inline uint16_t negate16(uint16_t value) {
    return (uint16_t)(0u - value);
}

// FUNCTION: GP4 0x004165c9
// Writes word +0xb4 of the record in esi: a magnitude derived from the word difference +0xe against
// [edi] (limit [0x603920] when it is not below [0x603926], else |[0x63803c] >> 10| between
// [0x603922] and [0x603920]), zero when [0x638d20] is zero and negated when it is negative.
void __cdecl FUN_004165c9(gp4::Regs* r) {
    Unknown_004165c9* self = reinterpret_cast<Unknown_004165c9*>(r->esi);
    const int16_t* other = reinterpret_cast<const int16_t*>(r->edi);

    if ((self->Flags6F & 1u) != 0u) {
        self->UnknownEE = 0x4000;
    }

    uint16_t cx = (uint16_t)GP4_GLOBAL(int16_t, 0x00603922);
    uint32_t edx = r->edx;
    uint16_t dx = (uint16_t)((uint16_t)self->Unknown0E - (uint16_t)*other);
    if ((int16_t)dx < 0) dx = negate16(dx);
    dx = (uint16_t)(dx << 1);
    if ((int16_t)dx < 0) dx = negate16(dx);
    dx = (uint16_t)(dx >> 1);
    edx = (edx & 0xffff0000u) | dx;

    if ((int16_t)dx >= GP4_GLOBAL(int16_t, 0x00603926)) {
        cx = (uint16_t)GP4_GLOBAL(int16_t, 0x00603920);
    } else {
        edx = (uint32_t)(GP4_GLOBAL(int32_t, 0x0063803c) >> 10);
        dx = (uint16_t)edx;
        if ((int16_t)dx < 0) {
            dx = negate16(dx);
            edx = (edx & 0xffff0000u) | dx;
        }
        if ((int16_t)dx >= (int16_t)cx) {
            cx = dx;
            if ((int16_t)cx > GP4_GLOBAL(int16_t, 0x00603920)) {
                cx = (uint16_t)GP4_GLOBAL(int16_t, 0x00603920);
            }
        }
    }

    uint16_t ax = cx;
    const uint16_t sign = (uint16_t)GP4_GLOBAL(int16_t, 0x00638d20);
    if (sign == 0u) {
        ax = 0;
    }
    uint32_t flags = testFlags16(GP4_GLOBAL(uint16_t, 0x00638d20));
    if ((int16_t)GP4_GLOBAL(int16_t, 0x00638d20) < 0) {
        flags = negFlags16(ax);
        ax = negate16(ax);
    }
    self->UnknownB4 = (int16_t)ax;

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004165c9, FUN_004165c9, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0xf8]:bytes, edi:ptr[8]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
