#include <gp4/gp4.h>

struct Unknown_004320b2 {
    uint8_t Unknown00[0x60];
    int16_t Unknown60;
    int16_t Unknown62;
    uint8_t Unknown64[0x9c];
    int32_t Unknown100;
    uint8_t Unknown104[8];
    int32_t Unknown10C;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

static inline uint32_t subFlags(uint32_t a, uint32_t b, uint32_t res) {
    return (a < b ? 0x001u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7)
        | ((((a ^ b) & (a ^ res)) >> 31) << 11);
}

static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t res) {
    return (res < a ? 0x001u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7)
        | ((((a ^ res) & (b ^ res)) >> 31) << 11);
}

// FUNCTION: GP4 0x004320b2
// Returns in eax an x offset built from the record in edi: the fixed-point
// (>> 8) field plus DAT_0062d2a4 plus a signed 16-bit field plus 0x50. When the
// sign bit of byte DAT_00868582 is set the left-hand fields (+0x100, +0x60) are
// used and the result is negated; otherwise the right-hand ones (+0x10c, +0x62).
// edx is preserved; the flags are those of the final neg / add.
void __cdecl FUN_004320b2(gp4::Regs* r) {
    const Unknown_004320b2* rec = reinterpret_cast<const Unknown_004320b2*>(static_cast<uintptr_t>(r->edi));
    uint32_t fl = r->eflags & ~0x8D5u;
    uint32_t eax;
    if (GP4_GLOBAL(int8_t, 0x00868582) < 0) {
        const uint32_t edx = (uint32_t)(int32_t)rec->Unknown60;
        eax = (uint32_t)(rec->Unknown100 >> 8);
        eax += GP4_GLOBAL(uint32_t, 0x0062d2a4);
        eax += edx;
        eax += 0x50u;
        const uint32_t res = 0u - eax;
        // neg: flags of 0 - eax
        fl |= subFlags(0u, eax, res);
        eax = res;
    } else {
        const uint32_t edx = (uint32_t)(int32_t)rec->Unknown62;
        eax = (uint32_t)(rec->Unknown10C >> 8);
        eax += GP4_GLOBAL(uint32_t, 0x0062d2a4);
        eax += edx;
        const uint32_t res = eax + 0x50u;
        fl |= addFlags(eax, 0x50u, res);
        eax = res;
    }
    r->eax = eax;
    r->eflags = fl;
}
GP4_IMPL(0x004320b2, FUN_004320b2, "regs(edi:ptr[0x118]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
