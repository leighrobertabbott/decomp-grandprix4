#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of a byte `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of `shl r32, 16` in the pinned oracle model: CF = last bit out, AF = 0,
// OF = bit31(value << 15 ^ result).
static inline uint32_t shl16Flags(uint32_t value) {
    const uint32_t result = value << 16;
    return ((value >> 16) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((value << 15) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00435945_Body {
    uint8_t Unknown00[0x3c];
    int32_t Unknown3C;
    uint8_t Unknown40[0x6a - 0x40];
    int16_t Unknown6A;
    uint8_t Unknown6C[0x78 - 0x6c];
    uint8_t Flags78;
    uint8_t Unknown79[0xe4 - 0x79];
    int8_t ModeE4;
    uint8_t UnknownE5;
    uint8_t FlagsE6;
    uint8_t UnknownE7[0x178 - 0xe7];
    uint8_t Unknown178;
    uint8_t Unknown179[0x180 - 0x179];
    int16_t Unknown180;
    uint8_t Unknown182[0x1d0 - 0x182];
    uint8_t Unknown1D0;
    uint8_t Unknown1D1[0x418 - 0x1d1];
    int16_t Blend418;
};

// FUNCTION: GP4 0x00435945
// Per-body steering/force smoothing step. eax starts as word +0x180 blended by 0x00435893. When
// bit 3 of +0xe6 is set it is mixed once more: eax = eax * q1 >> 14 + (word +0x6a * (byte +0x1d0 <<
// 6) / [0x006013ac]) * (0x4000 - q1) >> 14 with q1 = byte +0x178 (0 without bit 5 of +0x78) *
// [0x00603464] / 100. In mode +0xe4 = 2..5 the value is kept in ebx and 0x004358f8 blends the word
// +0x418 toward 0; the new word +0x180 is then |word +0x418| * |dword +0x3c| >> 16 divided by
// |ebx| (1 when zero), clamped to +-[0x006013ac], and dword +0x3c becomes that value << 16.
// Any other mode returns after 0x004358f8 with the flags of the failing byte compare; the 2..5
// path leaves the flags of the final "shl eax, 16".
void __cdecl FUN_00435945(gp4::Regs* r) {
    Unknown_00435945_Body* body = reinterpret_cast<Unknown_00435945_Body*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    s.eax = static_cast<uint32_t>(static_cast<int32_t>(body->Unknown180));
    gp4::call_regs(0x00435893, s);

    if ((body->FlagsE6 & 8u) != 0u) {
        const uint32_t blended = s.eax;
        uint32_t level = body->Unknown178;
        if ((body->Flags78 & 0x20u) == 0u) {
            level = 0;
        }
        int32_t remainder;
        const int64_t scaledLevel = static_cast<int64_t>(static_cast<int32_t>(level))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603464));
        s.ebp = static_cast<uint32_t>(gp4::idiv64(scaledLevel, 0x64, &remainder));

        const uint32_t scale = static_cast<uint32_t>(body->Unknown1D0) << 6;
        const int64_t lever = static_cast<int64_t>(static_cast<int32_t>(body->Unknown6A))
            * static_cast<int64_t>(static_cast<int32_t>(scale));
        s.eax = static_cast<uint32_t>(gp4::idiv64(lever, GP4_GLOBAL(int32_t, 0x006013ac), &remainder));
        s.edx = static_cast<uint32_t>(remainder);

        s.ebp = 0x4000u - s.ebp;
        const int64_t restPart = static_cast<int64_t>(static_cast<int32_t>(s.eax))
            * static_cast<int64_t>(static_cast<int32_t>(s.ebp));
        s.edx = static_cast<uint32_t>(static_cast<uint64_t>(restPart) >> 32);
        s.ecx = static_cast<uint32_t>(static_cast<uint64_t>(restPart) >> 14);
        s.ebp = 0x4000u - s.ebp;

        const int64_t blendPart = static_cast<int64_t>(static_cast<int32_t>(blended))
            * static_cast<int64_t>(static_cast<int32_t>(s.ebp));
        s.edx = static_cast<uint32_t>(static_cast<uint64_t>(blendPart) >> 32);
        s.eax = static_cast<uint32_t>(static_cast<uint64_t>(blendPart) >> 14) + s.ecx;
    }

    const int32_t mode = body->ModeE4;
    const bool active = mode >= 2 && mode <= 5;
    if (active) {
        s.ebx = s.eax;
        s.eax = 0;
    }
    gp4::call_regs(0x004358f8, s);

    uint32_t flags;
    if (!active) {
        flags = (mode < 2) ? cmpFlags8(static_cast<uint32_t>(mode), 2u) : cmpFlags8(static_cast<uint32_t>(mode), 5u);
    } else {
        s.eax = static_cast<uint32_t>(static_cast<int32_t>(body->Blend418));
        s.edx = static_cast<uint32_t>(body->Unknown3C);
        if (static_cast<int32_t>(s.edx) < 0) {
            s.edx = 0u - s.edx;
        }
        const int64_t torque = static_cast<int64_t>(static_cast<int32_t>(s.eax))
            * static_cast<int64_t>(static_cast<int32_t>(s.edx));
        const int64_t shifted = torque >> 16;
        if (s.ebx == 0u) {
            s.ebx = 1;
        }
        if (static_cast<int32_t>(s.ebx) < 0) {
            s.ebx = 0u - s.ebx;
        }
        int32_t remainder;
        const int32_t quotient = gp4::idiv64(shifted, static_cast<int32_t>(s.ebx), &remainder);
        s.edx = static_cast<uint32_t>(remainder);
        s.eax = static_cast<uint32_t>(quotient);

        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
        if (quotient < 0) {
            s.eax = 0u - s.eax;
            if (!(s.eax < limit)) {
                s.eax = limit;
            }
            s.eax = 0u - s.eax;
        } else if (!(s.eax < limit)) {
            s.eax = limit;
        }
        body->Unknown180 = static_cast<int16_t>(s.eax);
        flags = shl16Flags(s.eax);
        s.eax <<= 16;
        body->Unknown3C = static_cast<int32_t>(s.eax);
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00435945, FUN_00435945, "regs(ebx:u32, ecx:u32, ebp:u32, esi:ptr[0x420]:layout=rec_00435945) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=engine_00413606+state_00435945")
