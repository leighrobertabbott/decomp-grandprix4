#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
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

// Flags of a 32-bit CMP/SUB a - b.
static inline uint32_t flagsSub32(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    uint32_t f = parityFlag(res);
    if (a < b) f |= 0x1u;
    f |= (a ^ b ^ res) & 0x10u;
    if (res == 0u) f |= 0x40u;
    if (res & 0x80000000u) f |= 0x80u;
    if (((a ^ b) & (a ^ res)) & 0x80000000u) f |= 0x800u;
    return f;
}

// Flags of a 32-bit logic op (OR/TEST): CF = OF = AF = 0.
static inline uint32_t flagsLogic32(uint32_t res) {
    uint32_t f = parityFlag(res);
    if (res == 0u) f |= 0x40u;
    if (res & 0x80000000u) f |= 0x80u;
    return f;
}

static inline uint16_t abs16(uint16_t v) {
    return (v & 0x8000u) ? (uint16_t)(0u - v) : v;
}

static inline uint32_t setLow16(uint32_t reg, uint16_t v) {
    return (reg & 0xffff0000u) | v;
}

// FUNCTION: GP4 0x0042bb90
void __cdecl FUN_0042bb90(gp4::Regs* r) {
    uint8_t* self = (uint8_t*)r->esi;
    uint8_t* other = (uint8_t*)r->ebx;
    uint8_t* rec = (uint8_t*)r->edi;
    int16_t* limit = &GP4_FIELD(int16_t, self, 0x174);

    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    uint32_t flags;

    const uint16_t gap = (uint16_t)(GP4_GLOBAL(uint16_t, 0x00603e70) - GP4_GLOBAL(uint16_t, 0x00603ba0));
    ecx = setLow16(ecx, gap);

    bool computeLimit = false;
    if ((int16_t)gap >= 0) {
        bool checkSide = true;
        if (GP4_GLOBAL(uint8_t, 0x006077c9) & 0x20u) {
            const uint16_t side = (GP4_GLOBAL(uint8_t, 0x006077c9) & 0x08u) ? 0xffffu : 0u;
            ebp = setLow16(ebp, side);
        } else {
            const uint16_t d = (uint16_t)(GP4_FIELD(uint16_t, rec, 0x160) - GP4_FIELD(uint16_t, rec, 0x0));
            ebp = setLow16(ebp, d);
            const uint16_t ad = abs16(d);
            edx = setLow16(edx, ad);
            if ((int16_t)ad < GP4_GLOBAL(int16_t, 0x00622420)) {
                computeLimit = true;
                checkSide = false;
            }
        }
        if (checkSide) {
            const uint16_t e = (uint16_t)(GP4_FIELD(uint16_t, self, 0xdc) - GP4_FIELD(uint16_t, self, 0x90));
            edx = setLow16(edx, e);
            const bool eNeg = (int16_t)e < 0;
            const bool sideNeg = (int16_t)(uint16_t)ebp < 0;
            computeLimit = (eNeg == sideNeg);
        }
    }

    if (computeLimit) {
        const int16_t a = (int16_t)(GP4_FIELD(uint16_t, self, 0x9a) - GP4_FIELD(uint16_t, other, 0x9a));
        const uint16_t e = abs16((uint16_t)(GP4_FIELD(uint16_t, self, 0xdc) - GP4_FIELD(uint16_t, self, 0x90)));
        const int32_t prod = (int32_t)a * (int32_t)(int16_t)e;
        int32_t div = (int32_t)(int16_t)gap << 4;
        if (div < 0x40) div = 0x40;
        ecx = (uint32_t)div;
        int32_t rem;
        const int32_t q = gp4::idiv64((int64_t)prod, div, &rem);
        edx = (uint32_t)rem;
        int32_t v = ((q - 0x380) >> 4) + GP4_GLOBAL(int32_t, 0x00603c14);
        if (v > GP4_GLOBAL(int32_t, 0x00603c20)) v = GP4_GLOBAL(int32_t, 0x00603c20);
        if (v < GP4_GLOBAL(int32_t, 0x00603c14)) v = GP4_GLOBAL(int32_t, 0x00603c14);
        eax = (uint32_t)v;
        if (*limit < (int16_t)v) *limit = (int16_t)v;
    } else {
        if (*limit < 8) *limit = 8;
    }

    if (rec[0x51] & 0x20u) {
        const uint16_t sp = abs16(GP4_GLOBAL(uint16_t, 0x00603e70));
        edx = setLow16(edx, sp);
        if ((int16_t)sp < GP4_GLOBAL(int16_t, 0x00603db8)) {
            const uint16_t y = abs16(GP4_GLOBAL(uint16_t, 0x00604404));
            eax = setLow16(eax, y);
            if ((int16_t)y < GP4_GLOBAL(int16_t, 0x00603e68)) {
                const int32_t m = (int16_t)(uint16_t)(GP4_GLOBAL(uint16_t, 0x00603e68) - y);
                const int64_t p = (int64_t)m * (int64_t)GP4_GLOBAL(int32_t, 0x00603e64);
                edx = (uint32_t)((uint64_t)p >> 32);
                int32_t v = (int32_t)(uint32_t)((uint64_t)p >> 14);
                if (v < 8) v = 8;
                if (v > GP4_GLOBAL(int32_t, 0x00603c20)) v = GP4_GLOBAL(int32_t, 0x00603c20);
                eax = (uint32_t)v;
                if (*limit < (int16_t)v) *limit = (int16_t)v;
            }
        }
    }

    const int32_t s9a = GP4_FIELD(int16_t, self, 0x9a);
    const int32_t o9a = GP4_FIELD(int16_t, other, 0x9a);
    const int32_t sum = s9a + o9a;
    ebp = (uint32_t)sum;
    edx = (uint32_t)o9a;
    int32_t speed = GP4_GLOBAL(int16_t, 0x00603e70);
    eax = (uint32_t)speed;
    if (speed < 0) {
        flags = flagsLogic32((uint32_t)speed);
    } else {
        if (speed < GP4_GLOBAL(int32_t, 0x00603e08)) speed = GP4_GLOBAL(int32_t, 0x00603e08);
        const int64_t p = (int64_t)speed * (int64_t)sum;
        int32_t scale = (int32_t)(uint32_t)p >> 8;
        if (scale == 0) scale = 1;
        ebp = (uint32_t)scale;
        const int32_t diff = s9a - o9a;
        eax = (uint32_t)diff;
        edx = (uint32_t)o9a;
        if (diff < 0) {
            flags = flagsSub32((uint32_t)s9a, (uint32_t)o9a);
        } else {
            int32_t base = -(int32_t)GP4_FIELD(uint16_t, self, 0x9a);
            if (base < 0) base = 0;
            const int64_t p2 = (int64_t)(base + GP4_GLOBAL(int32_t, 0x00603e00)) * (int64_t)GP4_GLOBAL(int32_t, 0x00603dfc);
            const int32_t t = (int32_t)(uint32_t)((uint64_t)p2 >> 14) + GP4_GLOBAL(int32_t, 0x00603e04);
            int32_t rem;
            const int32_t q = gp4::idiv64((int64_t)t * (int64_t)diff, scale, &rem);
            ebp = (uint32_t)q;
            edx = (uint32_t)rem;
            const uint16_t y = abs16(GP4_GLOBAL(uint16_t, 0x00604404));
            eax = setLow16((uint32_t)q, y);
            const uint16_t e68 = GP4_GLOBAL(uint16_t, 0x00603e68);
            if (!((int16_t)y < (int16_t)e68)) {
                flags = flagsSub16(y, e68);
            } else {
                const int32_t m = (int16_t)(uint16_t)(e68 - y);
                const int64_t p3 = (int64_t)m * (int64_t)GP4_GLOBAL(int32_t, 0x00603e64);
                const int32_t f = (int32_t)(uint32_t)((uint64_t)p3 >> 14);
                const int64_t p4 = (int64_t)f * (int64_t)q;
                edx = (uint32_t)((uint64_t)p4 >> 32);
                int32_t v = (int32_t)(uint32_t)((uint64_t)p4 >> 14);
                if (v < 8) v = 8;
                if (v > GP4_GLOBAL(int32_t, 0x00603c20)) v = GP4_GLOBAL(int32_t, 0x00603c20);
                eax = (uint32_t)v;
                flags = flagsSub16((uint16_t)*limit, (uint16_t)v);
                if (*limit < (int16_t)v) *limit = (int16_t)v;
            }
        }
    }

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042bb90, FUN_0042bb90, "regs(esi:ptr[0x17c]:layout=rec_0042bb90, edi:ptr[0x168]:layout=link_0042bb90, ebx:ptr[0xa4]:layout=other_0042bb90, eax:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0042bb90")
