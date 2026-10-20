#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00404815
// Inverse-style companion of 0x00404796: stores (eax, edx) in 0x623e30 / 0x623e38, lets 0x0040460a
// translate and rotate them by the record at edi (result in 0x934df0 / 0x934df4), then derives
//   u    = (x * word[edi+0x14]) >> 14
//   gain = 0x40000 - u (signed floor 0x100)
//   edx  = ((y - (u >> 1)) << 14) / gain      (64-bit idiv, quotient)
// and returns x in eax. The four working globals 0x934de8..0x934df4 are saved and restored; ebx,
// ecx and ebp are preserved. The flags are those of the `shl eax, 14` feeding the idiv.
void __cdecl FUN_00404815(gp4::Regs* r) {
    const uint32_t inX = r->eax;
    const uint32_t inY = r->edx;
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const uint32_t savedCos = GP4_GLOBAL(uint32_t, 0x00934de8);
    const uint32_t savedSin = GP4_GLOBAL(uint32_t, 0x00934dec);
    const uint32_t savedX = GP4_GLOBAL(uint32_t, 0x00934df0);
    const uint32_t savedY = GP4_GLOBAL(uint32_t, 0x00934df4);

    GP4_GLOBAL(uint32_t, 0x00623e30) = inX;
    GP4_GLOBAL(uint32_t, 0x00623e38) = inY;

    gp4::Regs call = *r;
    gp4::call_regs(0x0040460a, call);

    const int32_t x = GP4_GLOBAL(int32_t, 0x00934df0);
    const int32_t k = GP4_FIELD(int16_t, rec, 0x14);
    const uint32_t u = static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(x) * k) >> 14);
    const uint32_t half = static_cast<uint32_t>(static_cast<int32_t>(u) >> 1);
    int32_t gain = static_cast<int32_t>(0x40000u - u);
    if (gain < 0x100) gain = 0x100;

    const uint32_t diff = GP4_GLOBAL(uint32_t, 0x00934df4) - half;
    const int64_t numerator = static_cast<int64_t>(static_cast<int32_t>(diff)) * 0x4000;
    int32_t rem = 0;
    const int32_t quotient = gp4::idiv64(numerator, gain, &rem);

    const uint32_t outX = static_cast<uint32_t>(x);

    GP4_GLOBAL(uint32_t, 0x00934df4) = savedY;
    GP4_GLOBAL(uint32_t, 0x00934df0) = savedX;
    GP4_GLOBAL(uint32_t, 0x00934dec) = savedSin;
    GP4_GLOBAL(uint32_t, 0x00934de8) = savedCos;

    // shl eax, 14 on `diff`: CF = last bit out, AF = 0 (pinned oracle model), OF = bit31((v << 13) ^ (v << 14))
    const uint32_t shifted = diff << 14;
    const uint32_t flags = ((diff >> 18) & 1u) | parityFlag8(shifted) | (shifted == 0u ? 0x40u : 0u)
        | ((shifted & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((diff << 13) ^ shifted) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = outX;
    r->edx = static_cast<uint32_t>(quotient);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00404815, FUN_00404815, "regs(eax:i32[-1048576..1048576], edx:i32[-1048576..1048576], edi:ptr[0x40]:i16[-3000..3000]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
