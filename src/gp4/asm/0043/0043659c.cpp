#include <gp4/gp4.h>

// The esi / edi blocks (only the fields this routine touches).
struct Unknown_0043659c_Block {
    int32_t Unknown00;
    int32_t Unknown04;
    uint16_t Unknown08;                   // 16-bit angle (0x4000 = quarter turn)
    uint8_t Unknown0a[0x0c - 0x0a];
    uint32_t Unknown0c;
    uint8_t Unknown10[0x14 - 0x10];
    uint32_t Unknown14;
    uint8_t Unknown18[0x20 - 0x18];
    uint32_t Unknown20;
};

// int16 table at 0x7c5c5c indexed by |angle| / 4 (word-aligned byte offset).
static inline int32_t tableLookup(uint16_t angle) {
    if ((int16_t)angle < 0) angle = (uint16_t)(0u - angle);
    const uint32_t offset = (uint32_t)(uint16_t)(angle >> 2) & 0xfffeu;
    return (int32_t)*(const int16_t*)(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
}

// Low 32 bits of (a * b) >> 14 (imul + shrd eax, edx, 14).
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t p = (int64_t)a * (int64_t)b;
    return (uint32_t)((uint64_t)p >> 14);
}

// Index of the first entry greater than value, or of the -1 terminator.
static inline int findBand(const int32_t* bounds, int32_t value) {
    int j = 0;
    while (!(value < bounds[j])) {
        j++;
        if (bounds[j] == -1) break;
    }
    return j;
}

// FUNCTION: GP4 0x0043659c
void __cdecl FUN_0043659c(gp4::Regs* r) {
    Unknown_0043659c_Block* const a = (Unknown_0043659c_Block*)r->edi;
    Unknown_0043659c_Block* const b = (Unknown_0043659c_Block*)r->esi;

    const int32_t dx = (int32_t)((uint32_t)b->Unknown00 - (uint32_t)a->Unknown00) >> 8;
    const int32_t dy = (int32_t)((uint32_t)b->Unknown04 - (uint32_t)a->Unknown04) >> 8;

    const uint16_t heading = a->Unknown08;
    GP4_GLOBAL(int32_t, 0x00629d98) = tableLookup(heading);
    GP4_GLOBAL(int32_t, 0x00629d9c) = tableLookup((uint16_t)(0x4000u - heading));
    const int32_t s0 = GP4_GLOBAL(int32_t, 0x00629d98);
    const int32_t c0 = GP4_GLOBAL(int32_t, 0x00629d9c);
    GP4_GLOBAL(uint32_t, 0x00629d88) = mulShr14(dx, s0) - mulShr14(dy, c0);
    GP4_GLOBAL(uint32_t, 0x00629d8c) = mulShr14(dy, s0) + mulShr14(dx, c0);

    const uint16_t relative = (uint16_t)(b->Unknown08 - a->Unknown08);
    GP4_GLOBAL(int32_t, 0x00629d90) = tableLookup(relative);
    GP4_GLOBAL(int32_t, 0x00629d94) = tableLookup((uint16_t)(0x4000u - relative));

    const int32_t* const px = GP4_ARRAY(const int32_t, 0x00629c68);
    const int32_t* const py = GP4_ARRAY(const int32_t, 0x00629c9c);
    const uint32_t* const bits = GP4_ARRAY(const uint32_t, 0x00629cd0);
    for (int i = 0; px[i] != -1; i++) {
        const int32_t s1 = GP4_GLOBAL(int32_t, 0x00629d90);
        const int32_t c1 = GP4_GLOBAL(int32_t, 0x00629d94);
        GP4_GLOBAL(uint32_t, 0x00629d80) =
            mulShr14(py[i], c1) + mulShr14(px[i], s1) + GP4_GLOBAL(uint32_t, 0x00629d88);
        GP4_GLOBAL(uint32_t, 0x00629d84) =
            (mulShr14(py[i], s1) - mulShr14(px[i], c1)) + GP4_GLOBAL(uint32_t, 0x00629d8c);

        const int j = findBand(GP4_ARRAY(const int32_t, 0x00629d00), GP4_GLOBAL(int32_t, 0x00629d80));
        const uint32_t rowMask = GP4_ARRAY(const uint32_t, 0x00629d40)[j];
        const int k = findBand(GP4_ARRAY(const int32_t, 0x00629d1c), GP4_GLOBAL(int32_t, 0x00629d84));
        const uint32_t mask = rowMask & GP4_ARRAY(const uint32_t, 0x00629d5c)[k];
        if (mask != 0) {
            a->Unknown0c |= mask;
            const uint32_t bit = bits[i];
            b->Unknown0c |= bit;
            if (GP4_GLOBAL(uint32_t, 0x00629be0) & mask) b->Unknown14 |= bit;
            if (GP4_GLOBAL(uint32_t, 0x00629be4) & mask) b->Unknown20 |= bit;
            if (GP4_GLOBAL(uint32_t, 0x00629be0) & bit) a->Unknown14 |= mask;
            if (GP4_GLOBAL(uint32_t, 0x00629be4) & bit) a->Unknown20 |= mask;
        }
    }

    // pushal/popal restores every GPR; the flags are those of the final
    // "cmp eax, -1" that found the terminator (equal: ZF and PF set).
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043659c, FUN_0043659c, "regs(edi:ptr[0x40]:layout=obj_0043659c, esi:ptr[0x40]:layout=obj_0043659c) -> (cf, pf, af, zf, sf, of)")
