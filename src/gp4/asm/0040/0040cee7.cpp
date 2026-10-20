#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0040cee7 {
    uint8_t Unknown00[0xec];
    int32_t UnknownEC;
    int32_t UnknownF0;
    uint8_t UnknownF4[4];
    int32_t UnknownF8;
};

// Rotates the vector (eax, 0) by the angle [edi+0xec] * [0x006f2bdc]: eax = round(eax*cos) +
// [edi+0xf0], edx = [edi+0xf8] - round(eax*sin). The flags are those of the final add into edx.
// FUNCTION: GP4 0x0040cee7
void __cdecl FUN_0040cee7(gp4::Regs* r) {
    const Unknown_0040cee7* body = reinterpret_cast<const Unknown_0040cee7*>(static_cast<uintptr_t>(r->edi));
    const int32_t length = static_cast<int32_t>(r->eax);

    const Ext angle = gp4::x87::ext_i32(body->UnknownEC) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    gp4::x87::fsincos(angle, &sine, &cosine);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = length;
    const int32_t across = gp4::x87::fistp32(cosine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = across;
    const uint32_t resultEax = static_cast<uint32_t>(across) + static_cast<uint32_t>(body->UnknownF0);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = length;
    const int32_t down = gp4::x87::fistp32(gp4::x87::fchs(sine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4))));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = down;
    const uint32_t base = static_cast<uint32_t>(body->UnknownF8);
    const uint32_t resultEdx = base + static_cast<uint32_t>(down);

    r->eax = resultEax;
    r->edx = resultEdx;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(base, static_cast<uint32_t>(down));
}
GP4_IMPL(0x0040cee7, FUN_0040cee7, "regs(eax:u32, edi:ptr[0x100]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:f64")
