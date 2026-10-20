#include <gp4/gp4.h>
extern "C" int __cdecl _div64(__int64 dividend, int divisor, int* remainder);  // compiler intrinsic: idiv

// imul by 0x6487ed then shrd 22: low dword of the 64-bit product >> 22.
static inline uint32_t ScaleBy6487ed(int32_t v) {
    const int64_t p = static_cast<int64_t>(v) * 0x6487ed;
    return static_cast<uint32_t>(static_cast<uint64_t>(p) >> 22);
}

// imul then shrd 14.
static inline uint32_t MulShr14(int32_t a, int32_t b) {
    const int64_t p = static_cast<int64_t>(a) * b;
    return static_cast<uint32_t>(static_cast<uint64_t>(p) >> 14);
}

// FUNCTION: GP4 0x004121c7
void __cdecl FUN_004121c7(gp4::Regs* r) {
    uint8_t* const base = GP4_ARRAY(uint8_t, 0x006380d8);

    const int32_t g334 = GP4_GLOBAL(int32_t, 0x00601334);
    const int32_t g338 = GP4_GLOBAL(int32_t, 0x00601338);
    const int32_t g340 = GP4_GLOBAL(int32_t, 0x00601340);
    const int32_t g34c = GP4_GLOBAL(int32_t, 0x0060134c);
    const int32_t g350 = GP4_GLOBAL(int32_t, 0x00601350);
    const int32_t g36c = GP4_GLOBAL(int32_t, 0x0060136c);

    const uint32_t half340 = static_cast<uint32_t>(g340 >> 1);
    GP4_FIELD(uint32_t, base, 0x94c) = half340;
    GP4_FIELD(uint32_t, base, 0x64c) = 0u - half340;
    const uint32_t half338 = static_cast<uint32_t>(g338 >> 1);
    GP4_FIELD(uint32_t, base, 0x34c) = half338;
    GP4_FIELD(uint32_t, base, 0x04c) = 0u - half338;
    const uint32_t neg34c = 0u - static_cast<uint32_t>(g34c);
    GP4_FIELD(uint32_t, base, 0x354) = neg34c;
    GP4_FIELD(uint32_t, base, 0x054) = neg34c;
    GP4_FIELD(uint32_t, base, 0x954) = static_cast<uint32_t>(g350);
    GP4_FIELD(uint32_t, base, 0x654) = static_cast<uint32_t>(g350);
    const uint32_t neg36c = 0u - static_cast<uint32_t>(g36c);
    GP4_FIELD(uint32_t, base, 0x050) = neg36c;
    GP4_FIELD(uint32_t, base, 0x350) = neg36c;
    GP4_FIELD(uint32_t, base, 0x650) = neg36c;
    GP4_FIELD(uint32_t, base, 0x950) = neg36c;

    GP4_GLOBAL(uint32_t, 0x00638da4) = ScaleBy6487ed(g34c);
    GP4_GLOBAL(uint32_t, 0x00638da8) = ScaleBy6487ed(g350);
    GP4_GLOBAL(uint32_t, 0x00638dac) = ScaleBy6487ed(GP4_GLOBAL(int32_t, 0x0060133c));
    GP4_GLOBAL(uint32_t, 0x00638db0) = ScaleBy6487ed(GP4_GLOBAL(int32_t, 0x00601344));
    GP4_GLOBAL(uint32_t, 0x00638db4) = ScaleBy6487ed(GP4_GLOBAL(int32_t, 0x00601348));

    const int32_t scale = GP4_GLOBAL(int32_t, 0x00601d68);
    int32_t* const d8c = GP4_ARRAY(int32_t, 0x00601d8c);
    uint32_t* const d9c = GP4_ARRAY(uint32_t, 0x00601d9c);
    const uint32_t* const db4 = GP4_ARRAY(const uint32_t, 0x00601db4);
    const uint32_t* const df4 = GP4_ARRAY(const uint32_t, 0x00601df4);
    const uint32_t* const e04 = GP4_ARRAY(const uint32_t, 0x00601e04);
    const int32_t* const e20 = GP4_ARRAY(const int32_t, 0x00601e20);
    const uint32_t* const e40 = GP4_ARRAY(const uint32_t, 0x00601e40);
    for (int i = 0; i < 4; ++i) {
        uint8_t* const rec = base + i * 0x300;
        const uint32_t v = MulShr14(d8c[i], scale);
        d9c[i] = v;
        GP4_FIELD(uint32_t, rec, 0x98) = v;
        GP4_FIELD(uint32_t, rec, 0x8c) = df4[i];
        GP4_FIELD(uint32_t, rec, 0x90) = e04[i];
        GP4_FIELD(uint32_t, rec, 0xa0) = db4[i];
        GP4_FIELD(uint32_t, rec, 0x1d0) = MulShr14(e20[i], scale);
        GP4_FIELD(uint32_t, rec, 0x1c8) = e40[i];
    }

    // cdq; shld/shl by 13; idiv -> 32-bit quotient (and remainder in edx).
    const int32_t ea4 = GP4_GLOBAL(int32_t, 0x00601ea4);
    int rem = 0;
    const int32_t d0 = static_cast<int32_t>(static_cast<uint32_t>(g338) - static_cast<uint32_t>(ea4));
    GP4_FIELD(int32_t, base, 0x074) = _div64(static_cast<int64_t>(d0) * 8192, g338, &rem);
    const int32_t d1 = static_cast<int32_t>(static_cast<uint32_t>(g338) + static_cast<uint32_t>(ea4));
    GP4_FIELD(int32_t, base, 0x374) = _div64(static_cast<int64_t>(d1) * 8192, g338, &rem);
    const int32_t d2 = static_cast<int32_t>(static_cast<uint32_t>(g340) - static_cast<uint32_t>(ea4));
    GP4_FIELD(int32_t, base, 0x674) = _div64(static_cast<int64_t>(d2) * 8192, g340, &rem);
    const int32_t d3 = static_cast<int32_t>(static_cast<uint32_t>(g340) + static_cast<uint32_t>(ea4));
    GP4_FIELD(int32_t, base, 0x974) = _div64(static_cast<int64_t>(d3) * 8192, g340, &rem);

    const uint32_t d4 = static_cast<uint32_t>(g350) - static_cast<uint32_t>(g34c);
    const int32_t q = _div64(static_cast<int64_t>(static_cast<int32_t>(d4)) * 16384, g334, &rem);
    GP4_GLOBAL(int32_t, 0x00601e3c) = q;

    r->eax = static_cast<uint32_t>(q);
    r->edx = static_cast<uint32_t>(rem);
    r->ebx = 0x006380d8u;
    r->ecx = 4u;
    r->ebp = 0x006487edu;

    // idiv leaves the flags of "shl eax, 14" (pinned Unicorn model):
    // CF = last bit out, PF of a zero low byte, AF = 0, OF = bit31(src<<13 ^ result).
    const uint32_t before = d4 << 13;
    const uint32_t shifted = d4 << 14;
    const uint32_t flags = ((before >> 31) & 1u)
        | 0x4u
        | (shifted == 0u ? 0x40u : 0u)
        | ((shifted & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((before ^ shifted) & 0x80000000u) != 0u) ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004121c7, FUN_004121c7, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_004121c7")
