#include <gp4/gp4.h>

// Keeps the compiler from folding state-relative offsets into absolute globals.
static __declspec(noinline) uint8_t* StateBase() { return GP4_ARRAY(uint8_t, 0x006380d8); }

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, n: the low 32 bits of the signed 64-bit product shifted right by n.
static inline uint32_t mulShr(uint32_t a, uint32_t b, int n) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> n);
}

// Gives the four wheel corner positions of the state at 0x006380d8 their offsets from the car frame.
// While DAT_00629bd0 is 0xff the two halves are the helpers 0x0041f1de (esi buffer, records at
// 0x006380d8) and 0x0041f28d. Otherwise the first half stores into the dwords +0x2fc..+0x308 of the
// esi buffer (the corner sums of DAT_00638034/38 with the four coefficients DAT_00638da4..db0, offset
// by DAT_00638044 - 0x6a00 and the top nibble of DAT_00638d58, minus +0x38c..+0x398) and the second
// into +0xc4, +0x3c4, +0x6c4 and +0x9c4 of the state from DAT_00602fcc/d4 and DAT_00638050 >> 4.
// ebx ends as 0x006380d8; eax, ecx, edx and ebp are the last values each half left; the flags are
// those of the second half (its final add, or the helper's).
// FUNCTION: GP4 0x0042045e
void __cdecl FUN_0042045e(gp4::Regs* r) {
    uint8_t* const buffer = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const state = StateBase();

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = 0x006380d8u;

    if (GP4_GLOBAL(uint8_t, 0x00629bd0) == 0xffu) {
        gp4::call_regs(0x0041f1de, c);
    } else {
        uint32_t base = GP4_GLOBAL(uint32_t, 0x00638044) + 0xffff9600u;
        base = (base << 4) | (GP4_GLOBAL(uint32_t, 0x00638d58) >> 28);
        const uint32_t u = base - mulShr(GP4_GLOBAL(uint32_t, 0x00638034), GP4_GLOBAL(uint32_t, 0x00638da4), 26);
        const uint32_t x2 = mulShr(GP4_GLOBAL(uint32_t, 0x00638038), GP4_GLOBAL(uint32_t, 0x00638dac), 26);
        GP4_FIELD(uint32_t, buffer, 0x300) = (u - x2) - GP4_FIELD(uint32_t, buffer, 0x390);
        GP4_FIELD(uint32_t, buffer, 0x2fc) = (x2 + u) - GP4_FIELD(uint32_t, buffer, 0x38c);
        const uint32_t v = mulShr(GP4_GLOBAL(uint32_t, 0x00638034), GP4_GLOBAL(uint32_t, 0x00638da8), 26) + base;
        const uint32_t x4 = mulShr(GP4_GLOBAL(uint32_t, 0x00638038), GP4_GLOBAL(uint32_t, 0x00638db0), 26);
        const uint32_t low = (v - x4) - GP4_FIELD(uint32_t, buffer, 0x398);
        GP4_FIELD(uint32_t, buffer, 0x308) = low;
        const uint32_t high = (x4 + v) - GP4_FIELD(uint32_t, buffer, 0x394);
        GP4_FIELD(uint32_t, buffer, 0x304) = high;
        c.ecx = base;
        c.ebp = v;
        c.edx = low;
        c.eax = high;
    }

    if (GP4_GLOBAL(uint8_t, 0x00629bd0) == 0xffu) {
        gp4::call_regs(0x0041f28d, c);
        c.eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
    } else {
        const uint32_t scaled = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00638050) >> 4);
        const uint32_t p = (0u - mulShr(GP4_GLOBAL(uint32_t, 0x00602fcc), GP4_GLOBAL(uint32_t, 0x00638da4), 18)) + scaled;
        const uint32_t y2 = mulShr(GP4_GLOBAL(uint32_t, 0x00602fd4), GP4_GLOBAL(uint32_t, 0x00638dac), 18);
        GP4_FIELD(uint32_t, state, 0x3c4) = p - y2;
        GP4_FIELD(uint32_t, state, 0xc4) = y2 + p;
        const uint32_t q = mulShr(GP4_GLOBAL(uint32_t, 0x00602fcc), GP4_GLOBAL(uint32_t, 0x00638da8), 18) + scaled;
        const uint32_t y4 = mulShr(GP4_GLOBAL(uint32_t, 0x00602fd4), GP4_GLOBAL(uint32_t, 0x00638db0), 18);
        GP4_FIELD(uint32_t, state, 0x9c4) = q - y4;
        GP4_FIELD(uint32_t, state, 0x6c4) = y4 + q;
        c.eax = y4 + q;
        c.edx = q - y4;
        c.ebp = q;
        c.eflags = (r->eflags & ~0x8d5u) | addFlags32(y4, q);
    }

    r->eax = c.eax;
    r->ebx = 0x006380d8u;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0042045e, FUN_0042045e, "regs(esi:ptr[0x3a8]:bytes, ecx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=spin_0041f28d")
