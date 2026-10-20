#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00526393
// Clamps the signed value in eax to 0x80..0x1f00 and stores it in 0x875278. Unless 0x874f44 == 1 it
// is then doubled and scaled by 0x875270 (Q14: low dword of (v*2 * scale) >> 14); that value goes
// to 0x875274 and drives the two products 0x8751f8/0x8751fc * value >> 8 stored in 0x875200 and
// 0x875204. eax/edx leave as the low/high dword of the last unsigned product (eax already shifted),
// with the flags of its `shrd eax, edx, 8` (pinned oracle model: AF = 0, OF = bit31(prev ^ result)
// where prev is the product shifted by 7).
void __cdecl FUN_00526393(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(r->eax);
    if (value > 0x1f00) {
        value = 0x1f00;
    }
    if (value < 0x80) {
        value = 0x80;
    }
    GP4_GLOBAL(int32_t, 0x00875278) = value;

    if (GP4_GLOBAL(int32_t, 0x00874f44) != 1) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(value) << 1)) *
                                static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00875270));
        value = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);
    }
    GP4_GLOBAL(int32_t, 0x00875274) = value;

    const uint64_t first = static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x008751f8)) *
                           static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00875274));
    GP4_GLOBAL(uint32_t, 0x00875200) = static_cast<uint32_t>(first >> 8);

    const uint64_t second = static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x008751fc)) *
                            static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00875274));
    const uint32_t result = static_cast<uint32_t>(second >> 8);
    GP4_GLOBAL(uint32_t, 0x00875204) = result;

    const uint32_t previous = static_cast<uint32_t>(second >> 7);
    const uint32_t flags = ((static_cast<uint32_t>(second) >> 7) & 1u) | gp4::flags::logic32(result) |
                           ((((previous ^ result) >> 31) & 1u) != 0u ? gp4::flags::OF : 0u);

    r->eax = result;
    r->edx = static_cast<uint32_t>(second >> 32);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00526393, FUN_00526393, "regs(eax:i32[-0x200..0x2400]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
