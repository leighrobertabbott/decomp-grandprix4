#include <gp4/gp4.h>

// Four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown000[0x18];
    int32_t Unknown018;
    int32_t Unknown01c;
    uint8_t Unknown020[0x0e];
    uint8_t Flags02e;
    uint8_t Unknown02f[0x149];
    int32_t Unknown178;
    uint8_t Unknown17c[0x14];
    int32_t Unknown190;
    int32_t Unknown194;
    uint8_t Unknown198[0x58];
    int32_t Unknown1f0;
    uint8_t Unknown1f4[0x10c];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// The record that esi points at (only the fields this routine touches).
struct Unknown_004178e0 {
    uint8_t Unknown00[0x40];
    uint32_t Flags40;
    uint8_t Unknown44[0x36];
    uint8_t Flags7a;
    uint8_t Unknown7b[0x05];
    uint8_t Flags80;
    uint8_t Flags81;
    uint8_t Unknown82[0x62];
    int8_t UnknownE4;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

static inline uint32_t compareFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = (uint8_t)(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of "neg eax" applied to value.
static inline uint32_t negFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((value & result & 0x80000000u) != 0u ? 0x800u : 0u);
}

// base of the record array that the original addresses through ebx
static __declspec(noinline) Unknown_006380d8* recordBase() {
    return GP4_ARRAY(Unknown_006380d8, 0x006380d8);
}

// Clamp |v| to the unsigned limit, keeping the sign.
static inline int32_t clampMagnitude(int32_t v, uint32_t limit) {
    if (v < 0) {
        uint32_t m = 0u - (uint32_t)v;
        if (!(m < limit)) m = limit;
        return (int32_t)(0u - m);
    }
    if (!((uint32_t)v < limit)) return (int32_t)limit;
    return v;
}

struct Outputs {
    uint32_t eax;
    uint32_t edx;
    uint32_t flags;
};

// -(v * scale >> 14), keeping the high product word for edx.
static inline uint32_t scaleNegate(int32_t v, int32_t scale, Outputs& o) {
    const int64_t p = (int64_t)v * (int64_t)scale;
    const uint32_t shifted = (uint32_t)((uint64_t)p >> 14);
    o.edx = (uint32_t)((uint64_t)p >> 32);
    o.flags = negFlags32(shifted);
    o.eax = 0u - shifted;
    return o.eax;
}

static inline void processPair(Unknown_004178e0* self, Unknown_006380d8* a, Unknown_006380d8* b,
                               uint8_t activeBit, uint8_t lowBit, int32_t scale, uint32_t limit,
                               uint32_t* outX, uint32_t* outY, Outputs& o) {
    if (((a->Flags02e | b->Flags02e) & 0x40u) == 0u) {
        o.flags = 0x44u;   // test byte, 0x40 with the bit clear
        return;
    }
    self->Flags80 |= activeBit;
    o.eax = (uint32_t)a->Unknown178;
    bool low = a->Unknown178 < 0x1e3c0000;
    if (!low) {
        o.eax = (uint32_t)b->Unknown178;
        low = b->Unknown178 < 0x1e3c0000;
    }
    if (low) {
        self->Flags81 |= lowBit;
    } else if (((a->Flags02e | b->Flags02e) & 0x20u) == 0u) {
        o.flags = 0x44u;   // test byte, 0x20 with the bit clear
        return;
    }

    const bool useFirst = (self->Flags7a & 0x20u) != 0u;
    int32_t x = useFirst ? (int32_t)((uint32_t)a->Unknown018 + (uint32_t)b->Unknown018) >> 1
                         : (int32_t)((uint32_t)a->Unknown190 + (uint32_t)b->Unknown190) >> 1;
    x = clampMagnitude(x, limit);
    const uint32_t vx = scaleNegate(x, scale, o);
    *outX = vx;
    const int32_t half = (int32_t)vx >> 1;
    a->Unknown1f0 = half;
    b->Unknown1f0 = half;

    int32_t y = useFirst ? (int32_t)((uint32_t)a->Unknown01c + (uint32_t)b->Unknown01c) >> 1
                         : (int32_t)((uint32_t)a->Unknown194 + (uint32_t)b->Unknown194) >> 1;
    y = clampMagnitude(y, limit);
    *outY = scaleNegate(y, scale, o);
}

// FUNCTION: GP4 0x004178e0
void __cdecl FUN_004178e0(gp4::Regs* r) {
    Unknown_004178e0* self = (Unknown_004178e0*)r->esi;
    Unknown_006380d8* rec = recordBase();

    self->Flags80 &= 0xfcu;
    self->Flags81 &= 0x3fu;
    GP4_GLOBAL(uint32_t, 0x00601fec) = 0u;
    GP4_GLOBAL(uint32_t, 0x00601fe8) = 0u;
    GP4_GLOBAL(uint32_t, 0x00601ff4) = 0u;
    GP4_GLOBAL(uint32_t, 0x00601ff0) = 0u;

    int32_t scale = GP4_GLOBAL(int32_t, 0x00601dc8);
    if ((self->Flags40 & 0x14280u) != 0u) scale = GP4_GLOBAL(int32_t, 0x00601dc4);
    GP4_GLOBAL(int32_t, 0x00601dcc) = scale;

    Outputs o;
    o.eax = (uint32_t)scale;
    o.edx = r->edx;

    const int8_t mode = self->UnknownE4;
    if (mode >= 2 && mode <= 5) {
        o.flags = compareFlags8((uint8_t)mode, 5u);
    } else {
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00601dd0);
        processPair(self, &rec[0], &rec[1], 0x01u, 0x40u, scale, limit,
                    &GP4_GLOBAL(uint32_t, 0x00601fe8), &GP4_GLOBAL(uint32_t, 0x00601ff0), o);
        processPair(self, &rec[2], &rec[3], 0x02u, 0x80u, scale, limit,
                    &GP4_GLOBAL(uint32_t, 0x00601fec), &GP4_GLOBAL(uint32_t, 0x00601ff4), o);
    }

    r->eax = o.eax;
    r->ebx = 0x006380d8u;
    r->edx = o.edx;
    r->eflags = (r->eflags & ~0x8d5u) | o.flags;
}
GP4_IMPL(0x004178e0, FUN_004178e0, "regs(esi:ptr[0xec]:layout=rec_004178e0, edx:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=state_004178e0")
