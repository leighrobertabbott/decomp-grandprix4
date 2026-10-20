#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `test dword, -1` (CF = OF = AF = 0).
static inline uint32_t testFlags(uint32_t value) {
    return parityFlag8(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);
}

// Flags of `sub dword, imm`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Rebases a non-null pointer field of the car record from absolute to an offset from the car array
// 0x0066feec; returns the flags of the last test/sub pair.
static inline uint32_t rebaseField(uint8_t* record, uint32_t offset) {
    uint32_t& field = GP4_FIELD(uint32_t, record, offset);
    if (field == 0u) return testFlags(field);
    const uint32_t before = field;
    field = before - 0x0066feecu;
    return subFlags(before, 0x0066feecu);
}

// FUNCTION: GP4 0x0040b0da
// Converts the absolute pointers of a 0x41c-byte car record into offsets: the two fields at +0x1c
// and +0x1b8 are rebased against 0x0089423c, the seven optional fields +0x114, +0x118, +0x110,
// +0x128, +0x12c, +0x130, +0x124 (when non-zero) against 0x0066feec. Only the flags change in the
// registers: they are those of the last test/sub (field +0x124).
void __cdecl FUN_0040b0da(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_FIELD(uint32_t, record, 0x1c) -= 0x0089423cu;
    GP4_FIELD(uint32_t, record, 0x1b8) -= 0x0089423cu;
    rebaseField(record, 0x114);
    rebaseField(record, 0x118);
    rebaseField(record, 0x110);
    rebaseField(record, 0x128);
    rebaseField(record, 0x12c);
    rebaseField(record, 0x130);
    const uint32_t flags = rebaseField(record, 0x124);

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040b0da, FUN_0040b0da, "regs(esi:ptr[0x1c0]:i32[0..1]) -> (cf, pf, af, zf, sf, of)")
