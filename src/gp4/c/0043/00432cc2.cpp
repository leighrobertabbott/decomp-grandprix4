#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `sar value, n` (1 < n < 32): CF = last bit shifted out, OF = AF = 0.
static inline uint32_t sarFlags(int32_t value, uint32_t n) {
    const int32_t result = value >> n;
    return ((static_cast<uint32_t>(value) >> (n - 1u)) & 1u)
        | parityFlag(static_cast<uint32_t>(result))
        | (result == 0 ? 0x40u : 0u)
        | (result < 0 ? 0x80u : 0u);
}

// One channel update: asks 0x00433ff8 about `slot`; when its carry is set, entry `eax` of the
// 8-byte table at 0x0062ff58 (float value, int divisor) is scaled to a fixed-point position
// stored at body+posOffset, and the change against the word at body+prevOffset is turned into a
// rate stored at body+rateOffset. Updates eax/edx/ebp/flags in `r` like the original.
static inline void updateChannel(gp4::Regs* r, uint8_t* body, uint32_t slot, uint32_t posOffset,
                                 uint32_t prevOffset, uint32_t rateOffset) {
    using namespace gp4::x87;

    gp4::Regs callee = *r;
    callee.eax = slot;
    gp4::call_regs(0x00433ff8, callee);
    r->eax = callee.eax;
    r->edx = callee.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
    if ((callee.eflags & 1u) == 0u) {
        return;
    }

    uint8_t* table = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uintptr_t, 0x0062ff58));
    const uint32_t index = callee.eax;
    const uint32_t raw = GP4_FIELD(uint32_t, table, index * 8u);
    int32_t divisor = GP4_FIELD(int32_t, table, index * 8u + 4u);
    if (divisor == 0) {
        divisor = 1;
    }

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = raw;
    const Ext scaled = ext_f32(GP4_GLOBAL(float, 0x006f2bf4))
        * ext(GP4_GLOBAL(double, 0x006f2af4))
        * ext(GP4_GLOBAL(double, 0x00601400));
    const int32_t fixed = fistp32(scaled);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fixed;

    const int32_t position = fixed >> 10;
    GP4_FIELD(int16_t, body, posOffset) = static_cast<int16_t>(position);

    const int32_t delta = position - static_cast<int32_t>(GP4_FIELD(int16_t, body, prevOffset));
    const int32_t scaledDelta = static_cast<int32_t>(
        static_cast<uint32_t>(delta) * static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00603854)));
    const int64_t product = static_cast<int64_t>(scaledDelta) * 1000;
    const int64_t dividend = product >> 16;
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, divisor, &remainder);
    GP4_FIELD(int16_t, body, rateOffset) = static_cast<int16_t>(quotient);

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = static_cast<uint32_t>(divisor);
    // idiv leaves the flags of the preceding `sar edx, 16`
    r->eflags = (r->eflags & ~0x8d5u) | sarFlags(static_cast<int32_t>(static_cast<uint64_t>(product) >> 32), 16u);
}

// FUNCTION: GP4 0x00432cc2
// Updates the two channels of the record at esi: slot 1 writes the words at +0x27a / +0x27c
// (previous position at +0x278), slot 2 writes +0x280 / +0x282 (previous at +0x27e).
void __cdecl FUN_00432cc2(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    updateChannel(r, body, 1u, 0x27au, 0x278u, 0x27cu);
    updateChannel(r, body, 2u, 0x280u, 0x27eu, 0x282u);
}
GP4_IMPL(0x00432cc2, FUN_00432cc2, "regs(esi:ptr[0x290]:bytes, ebp:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00432cc2")
