#include <gp4/gp4.h>

static inline uint32_t ParityFlag(uint32_t v) {
    uint32_t b = v & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;
}

// Q14 cosine/sine word table at 0x007c5c5c, indexed by |angle| >> 2 (word aligned).
static inline uint32_t TableWord(uint16_t angle) {
    uint16_t a = angle;
    if ((int16_t)a < 0) a = (uint16_t)(0u - a);
    a = (uint16_t)(a >> 2);
    const uint32_t off = (uint32_t)a & 0xfffeu;
    return (uint32_t)(int32_t) * reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + off);
}

static inline uint32_t MulShr(uint32_t a, uint32_t b, int sh) {
    return (uint32_t)((uint64_t)((int64_t)(int32_t)a * (int64_t)(int32_t)b) >> sh);
}

// FUNCTION: GP4 0x0041c622
// Stores (eax, edx) in 0x623e30 / 0x623e38 and lets 0x0040460a map them into the frame of the record at
// edi (result x/y in 0x934df0 / 0x934df4). If x lies within [-(w60+0x50)<<8, (w62+0x50)<<8] the result is
//   eax = x, edx = y - ((x * word[+0x14]) >> 15), ecx = 0.
// Otherwise the point is re-mapped relative to the end point (w4*8 + wD4/wD6, w8*8 + wDC/wDE) << 8 and
// rotated by the end angle (wD0 for the negative side, wD2 for the positive side):
//   edx = (c*dx + s*dy) >> 14, eax = (s*dx - c*dy) >> 14, ecx = -1 / +1, ebp = c (edi, esi, ebx preserved).
// CF = 1 when edx >= 0 (stc), else 0 (clc); the other flags are those of "or edx, edx".
void __cdecl FUN_0041c622(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(uint32_t, 0x00623e30) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00623e38) = r->edx;

    gp4::Regs call = *r;
    gp4::call_regs(0x0040460a, call);

    const uint32_t x = GP4_GLOBAL(uint32_t, 0x00934df0);
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x00934df4);

    uint32_t outEax = 0, outEdx = 0, outEcx = 0;
    bool rotate = false;
    uint32_t endOffX = 0, endOffY = 0, angleOff = 0;

    if ((int32_t)x >= 0) {
        const uint32_t limit = ((uint32_t)GP4_FIELD(uint16_t, rec, 0x62) + 0x50u) << 8;
        if ((int32_t)x > (int32_t)limit) {
            rotate = true;
            endOffX = 0xd6u;
            endOffY = 0xdeu;
            angleOff = 0xd2u;
            outEcx = 1u;
        }
    } else {
        const uint32_t limit = (0u - ((uint32_t)GP4_FIELD(uint16_t, rec, 0x60) + 0x50u)) << 8;
        if ((int32_t)x < (int32_t)limit) {
            rotate = true;
            endOffX = 0xd4u;
            endOffY = 0xdcu;
            angleOff = 0xd0u;
            outEcx = 0xffffffffu;
        }
    }

    if (!rotate) {
        const uint32_t lean = MulShr((uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 0x14), x, 15);
        outEax = x;
        outEdx = y - lean;
        outEcx = 0u;
    } else {
        const uint32_t baseX = (uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 4) << 3;
        const uint32_t dx = GP4_GLOBAL(uint32_t, 0x00623e30) -
                            (((uint32_t)(int32_t)GP4_FIELD(int16_t, rec, endOffX) + baseX) << 8);
        const uint32_t baseY = (uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 8) << 3;
        const uint32_t dy = GP4_GLOBAL(uint32_t, 0x00623e38) -
                            (((uint32_t)(int32_t)GP4_FIELD(int16_t, rec, endOffY) + baseY) << 8);
        const uint16_t angle = GP4_FIELD(uint16_t, rec, angleOff);
        const uint32_t c = TableWord((uint16_t)(0x4000u - angle));
        const uint32_t s = TableWord(angle);
        outEdx = MulShr(c, dx, 14) + MulShr(s, dy, 14);
        outEax = MulShr(s, dx, 14) - MulShr(c, dy, 14);
        r->ebp = c;
    }

    // or edx, edx: CF = OF = AF = 0; then stc when SF clear, clc when set.
    const uint32_t sf = outEdx >> 31;
    const uint32_t flags = (sf ? 0u : 1u) | ParityFlag(outEdx) | (outEdx == 0u ? 0x40u : 0u) | (sf << 7);
    r->eflags = (r->eflags & ~0x8d5u) | flags;

    r->eax = outEax;
    r->edx = outEdx;
    r->ecx = outEcx;
}
GP4_IMPL(0x0041c622, FUN_0041c622, "regs(eax:i32[-0x100000..0x100000], edx:i32[-0x100000..0x100000], edi:ptr[0xe8]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
