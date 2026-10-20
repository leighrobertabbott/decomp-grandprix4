#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

struct Unknown_0040b629 {
    uint8_t Unknown00[0x7a];
    uint16_t Unknown7A;
};

// Resets the 6-byte entry (10-byte stride, base 0x675a82) selected by the word at edi+0x7a:
// bytes +0, +1, +3, +4, +5 become 0 and byte +2 becomes 0x3f. eax and edi are saved and restored;
// the flags are those of the "add edi, 0x675a82" (the stores do not touch them).
// FUNCTION: GP4 0x0040b629
void __cdecl FUN_0040b629(gp4::Regs* r) {
    const Unknown_0040b629* rec = reinterpret_cast<const Unknown_0040b629*>(static_cast<uintptr_t>(r->edi));
    const uint32_t offset = static_cast<uint32_t>(rec->Unknown7A) * 10u;
    const uint32_t entry = offset + 0x00675a82u;
    uint8_t* bytes = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(entry));

    bytes[2] = 0x3f;
    bytes[4] = 0;
    bytes[5] = 0;
    bytes[3] = 0;
    bytes[0] = 0;
    bytes[1] = 0;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(offset, 0x00675a82u, entry);
}
GP4_IMPL(0x0040b629, FUN_0040b629, "regs(edi:ptr[0x82]:bytes) -> (cf, pf, af, zf, sf, of)")
