#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// imul (one operand) followed by shrd eax, edx, 16: low 32 bits of the 64-bit product >> 16
static inline uint32_t mulShift16(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * b) >> 16);
}

// FUNCTION: GP4 0x00433620
void __cdecl FUN_00433620(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00601bd0);

    // clamp the scaled +0xac field to +-0x400000 (sign-magnitude), then shift into 24.8
    uint32_t term = mulShift16(GP4_FIELD(int32_t, body, 0xac), scale);
    if (static_cast<int32_t>(term) < 0) {
        uint32_t magnitude = 0u - term;
        if (magnitude >= 0x400000u) {
            magnitude = 0x400000u;
        }
        term = 0u - magnitude;
    } else if (term >= 0x400000u) {
        term = 0x400000u;
    }
    const uint32_t ecxOut = r->ecx + (term << 8);

    const uint32_t first = mulShift16(GP4_GLOBAL(int32_t, 0x00601bc4), scale) + GP4_GLOBAL(uint32_t, 0x00601bbc);
    const uint32_t secondTerm = mulShift16(GP4_GLOBAL(int32_t, 0x00601bc8), scale);
    const uint32_t secondBase = GP4_GLOBAL(uint32_t, 0x00601bc0);
    const uint32_t second = secondTerm + secondBase;

    r->eax = first;
    r->edx = second;
    r->ecx = ecxOut;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(secondTerm, secondBase);
}
GP4_IMPL(0x00433620, FUN_00433620, "regs(ecx:u32, esi:ptr[0xb4]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
