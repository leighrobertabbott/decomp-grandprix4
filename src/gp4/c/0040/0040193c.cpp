#include <gp4/gp4.h>

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

// low 32 bits of the signed product (what `imul m32` leaves in eax)
static inline int32_t low32(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b));
}

// FUNCTION: GP4 0x0040193c
// Builds the Q14 3x3 rotation matrix at 0x00623e60 (rows of three dwords, 0x00623e60..0x00623e80)
// from the sine/cosine pairs at 0x00623edc/e0, 0x00623ee4/e8 and 0x00623eec/f0 (A..F below).
// Every product is truncated to 32 bits and shifted on its own (`imul m32; sar eax, 14`).
// esi and edi are preserved; the original leaves eax = the last term, edx = the high half of
// the last product (0x00623ee0 * 0x00623ee8), ebp = low32(A * E) and the flags of the final sar.
void __cdecl FUN_0040193c(gp4::Regs* r) {
    const int32_t a = GP4_GLOBAL(int32_t, 0x00623edc);
    const int32_t b = GP4_GLOBAL(int32_t, 0x00623ee0);
    const int32_t c = GP4_GLOBAL(int32_t, 0x00623ee4);
    const int32_t d = GP4_GLOBAL(int32_t, 0x00623ee8);
    const int32_t e = GP4_GLOBAL(int32_t, 0x00623eec);
    const int32_t f = GP4_GLOBAL(int32_t, 0x00623ef0);

    const int32_t ce = low32(c, e) >> 14;
    const int32_t cf = low32(c, f) >> 14;

    GP4_GLOBAL(int32_t, 0x00623e60) = (low32(b, f) + low32(a, ce)) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e64) = static_cast<int32_t>(0u - static_cast<uint32_t>(low32(d, e) >> 14));
    GP4_GLOBAL(int32_t, 0x00623e68) = (low32(b, ce) - low32(a, f)) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e6c) = (low32(b, e) - low32(a, cf)) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e70) = low32(d, f) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e74) =
        static_cast<int32_t>(0u - static_cast<uint32_t>((low32(b, cf) + low32(a, e)) >> 14));
    GP4_GLOBAL(int32_t, 0x00623e78) = low32(a, d) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e7c) = c;

    const int32_t last = low32(b, d);
    GP4_GLOBAL(int32_t, 0x00623e80) = last >> 14;

    const int64_t product = static_cast<int64_t>(b) * static_cast<int64_t>(d);
    r->eax = static_cast<uint32_t>(last >> 14);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebp = static_cast<uint32_t>(low32(a, e));
    r->eflags = (r->eflags & ~0x8d5u) | sarFlags(last, 14u);
}
GP4_IMPL(0x0040193c, FUN_0040193c, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
