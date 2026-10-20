#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of a logic op (TEST/OR/XOR): CF = OF = AF = 0.
static inline uint32_t flagsLogic8(uint8_t res) {
    uint32_t f = parityFlag(res);
    if (res == 0u) f |= 0x40u;
    if (res & 0x80u) f |= 0x80u;
    return f;
}

static inline uint32_t flagsLogic16(uint16_t res) {
    uint32_t f = parityFlag(res);
    if (res == 0u) f |= 0x40u;
    if (res & 0x8000u) f |= 0x80u;
    return f;
}

static inline uint32_t flagsLogic32(uint32_t res) {
    uint32_t f = parityFlag(res);
    if (res == 0u) f |= 0x40u;
    if (res & 0x80000000u) f |= 0x80u;
    return f;
}

// Flags of a 16-bit CMP/SUB a - b.
static inline uint32_t flagsSub16(uint16_t a, uint16_t b) {
    const uint16_t res = (uint16_t)(a - b);
    uint32_t f = parityFlag(res);
    if (a < b) f |= 0x1u;
    f |= (uint32_t)((a ^ b ^ res) & 0x10u);
    if (res == 0u) f |= 0x40u;
    if (res & 0x8000u) f |= 0x80u;
    if (((a ^ b) & (a ^ res)) & 0x8000u) f |= 0x800u;
    return f;
}

// Flags of a 16-bit ADD a + b.
static inline uint32_t flagsAdd16(uint16_t a, uint16_t b) {
    const uint16_t res = (uint16_t)(a + b);
    uint32_t f = parityFlag(res);
    if (res < a) f |= 0x1u;
    f |= (uint32_t)((a ^ b ^ res) & 0x10u);
    if (res == 0u) f |= 0x40u;
    if (res & 0x8000u) f |= 0x80u;
    if (((a ^ res) & (b ^ res)) & 0x8000u) f |= 0x800u;
    return f;
}

static inline uint32_t setLow16(uint32_t reg, uint16_t v) {
    return (reg & 0xffff0000u) | v;
}

// FUNCTION: GP4 0x0042bdec
void __cdecl FUN_0042bdec(gp4::Regs* r) {
    uint8_t* self = (uint8_t*)r->esi;
    const uint8_t* seg = (const uint8_t*)GP4_FIELD(uint32_t, self, 0x1c);

    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    uint32_t flags = 0;

    const uint8_t segFlags = seg[0x51];
    const int8_t dir = GP4_GLOBAL(int8_t, 0x00604155);

    do {
        if (!(segFlags & 0x20u)) { flags = flagsLogic8((uint8_t)(segFlags & 0x20u)); break; }
        if (segFlags & 0x80u) { flags = flagsLogic8((uint8_t)(segFlags & 0x80u)); break; }

        const uint16_t dir16 = (uint16_t)(int16_t)dir;
        const uint32_t dirExt = (uint32_t)(int32_t)dir;

        const uint16_t w184 = GP4_FIELD(uint16_t, self, 0x184);
        const uint16_t x = (uint16_t)(dir16 ^ w184);
        edx = setLow16(dirExt, x);
        if (x & 0x8000u) { flags = flagsLogic16(x); break; }

        const uint16_t w90 = GP4_FIELD(uint16_t, self, 0x90);
        const uint16_t y = (uint16_t)(dir16 ^ w90);
        eax = setLow16(eax, w90);
        edx = setLow16(dirExt, y);
        if (y & 0x8000u) {
            uint16_t a = w90;
            if (a & 0x8000u) a = (uint16_t)(0u - a);
            a = (uint16_t)(a + 0x40u);
            eax = setLow16(eax, a);
            const int16_t limit = GP4_FIELD(int16_t, self, 0xb0);
            if ((int16_t)a >= limit) { flags = flagsSub16(a, (uint16_t)limit); break; }
        }

        const int32_t diff = (int16_t)(uint16_t)(GP4_GLOBAL(uint16_t, 0x006077c5) - GP4_GLOBAL(uint16_t, 0x00604402));
        eax = (uint32_t)diff;
        if (!(segFlags & 0x08u)) {
            if (diff < 0) { flags = flagsLogic32(eax); break; }
            if (dir < 0) { flags = flagsLogic8((uint8_t)dir); break; }
        } else {
            if (diff >= 0) { flags = flagsLogic32(eax); break; }
            if (dir >= 0) { flags = flagsLogic8((uint8_t)dir); break; }
        }

        int32_t divisor = GP4_FIELD(int16_t, self, 0xde);
        if (divisor < 0x10) divisor = 0x10;
        ebp = (uint32_t)divisor;
        int32_t rem;
        int32_t q = gp4::idiv64((int64_t)diff * (int64_t)GP4_GLOBAL(int32_t, 0x006f2bf0), divisor, &rem);
        if (q < 0) q = (int32_t)(0u - (uint32_t)q);
        const uint32_t pushed = (uint32_t)q;

        const int64_t prod = (int64_t)GP4_FIELD(int32_t, self, 0x284) * (int64_t)GP4_GLOBAL(int32_t, 0x00603854);
        const uint32_t hi = (uint32_t)((uint64_t)prod >> 32);
        const uint32_t shr = ((uint32_t)prod >> 24) | (hi << 8);

        uint16_t w = GP4_FIELD(uint16_t, self, 0x184);
        if (w & 0x8000u) w = (uint16_t)(0u - w);
        w = (uint16_t)(0u - w);
        const uint16_t sum = (uint16_t)(w + (uint16_t)shr);
        edx = setLow16(hi, sum);
        eax = pushed;
        if (!(sum & 0x8000u)) { flags = flagsAdd16(w, (uint16_t)shr); break; }

        flags = flagsSub16((uint16_t)pushed, sum);
        if ((int16_t)pushed <= (int16_t)sum) break;

        self[0xca] |= 0x80u;
        const int16_t cur = GP4_FIELD(int16_t, self, 0x174);
        flags = flagsSub16((uint16_t)cur, 0x60u);
        if (cur < 0x60) GP4_FIELD(int16_t, self, 0x174) = 0x60;
    } while (0);

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042bdec, FUN_0042bdec, "regs(esi:ptr[0x2a4]:layout=rec_0042a9cf, eax:u32, edx:u32, ebp:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
