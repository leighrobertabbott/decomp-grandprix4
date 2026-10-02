#include <gp4/gp4.h>

struct Unknown_004384a3_Edi {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
    uint8_t Unknown64[0x9c];
    int32_t Unknown100;
    uint8_t Unknown104[8];
    int32_t Unknown10C;
};

struct Unknown_004384a3_Esi {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
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

// FUNCTION: GP4 0x004384a3
// Returns in eax an x offset: the fixed-point (>> 8) field of the record in edi
// plus its unsigned 16-bit field plus 0x50, then plus the signed 16-bit field at
// esi+0x90 (or minus it when the sign bit of byte DAT_00868582 is set, in which
// case the +0x10c / +0x62 fields are used instead of +0x100 / +0x60).
// edx is preserved; the flags are those of the final add / sub.
void __cdecl FUN_004384a3(gp4::Regs* r) {
    const Unknown_004384a3_Edi* rec = reinterpret_cast<const Unknown_004384a3_Edi*>(static_cast<uintptr_t>(r->edi));
    const Unknown_004384a3_Esi* other = reinterpret_cast<const Unknown_004384a3_Esi*>(static_cast<uintptr_t>(r->esi));
    uint32_t fl = r->eflags & ~0x8D5u;
    uint32_t eax;
    if (GP4_GLOBAL(int8_t, 0x00868582) >= 0) {
        eax = (uint32_t)(rec->Unknown100 >> 8);
        eax += (uint32_t)rec->Unknown60;
        eax += 0x50u;
        const uint32_t edx = (uint32_t)(int32_t)other->Unknown90;
        const uint32_t res = eax + edx;
        fl |= addFlags(eax, edx, res);
        eax = res;
    } else {
        eax = (uint32_t)(rec->Unknown10C >> 8);
        eax += (uint32_t)rec->Unknown62;
        eax += 0x50u;
        const uint32_t edx = (uint32_t)(int32_t)other->Unknown90;
        const uint32_t res = eax - edx;
        fl |= subFlags(eax, edx, res);
        eax = res;
    }
    r->eax = eax;
    r->eflags = fl;
}
GP4_IMPL(0x004384a3, FUN_004384a3, "regs(edi:ptr[0x118]:bytes, esi:ptr[0x98]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
