#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004124a4
void __cdecl FUN_004124a4(gp4::Regs* r) {
    // [0x601c74] = 0x4000 - (min(|[0x601c70]|, d) * [0x601c6c]) / d with d = dword [esi+0x16c]
    // (1 when zero). ebp keeps d, edx the idiv remainder, eax the stored value; the flags are
    // those of the final `add eax, 0x4000`.
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(int32_t, 0x00601c74) = 0x4000;
    int32_t divisor = GP4_FIELD(int32_t, body, 0x16c);
    if (divisor == 0) divisor = 1;
    int32_t value = GP4_GLOBAL(int32_t, 0x00601c70);
    if (value < 0) value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    if (value > divisor) value = divisor;
    const int64_t product = static_cast<int64_t>(value) * GP4_GLOBAL(int32_t, 0x00601c6c);
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(product, divisor, &remainder);
    const uint32_t negated = 0u - static_cast<uint32_t>(quotient);
    const uint32_t result = negated + 0x4000u;
    GP4_GLOBAL(uint32_t, 0x00601c74) = result;
    r->eax = result;
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = static_cast<uint32_t>(divisor);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(negated, 0x4000u);
}
GP4_IMPL(0x004124a4, FUN_004124a4, "regs(esi:ptr[0x174]:layout=rec_004124a4) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
