#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `test byte ptr [m], imm8`: CF = OF = AF = 0, PF/ZF/SF from the 8-bit result.
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of `test dword ptr [m], 0xffffffff`.
static inline uint32_t testFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0042a9cf_Owner {
    uint8_t Unknown00[0x12];
    uint8_t Flags12;
    uint8_t Unknown13[0x51 - 0x13];
    uint8_t Flags51;
};

struct Unknown_0042a9cf_Subject {
    uint8_t Unknown00[0x1c];
    Unknown_0042a9cf_Owner* Unknown1C;
    uint8_t Unknown20[0x8d - 0x20];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xce - 0x8e];
    uint8_t FlagsCE;
    uint8_t UnknownCF[0x1ce - 0xcf];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x29c - 0x1cf];
    int32_t Unknown29C;
};

// The stc/clc epilogue REPLACES CF; PF/AF/ZF/SF/OF stay from the last flag writer.
static inline void finish(gp4::Regs* r, uint32_t flags, bool carry) {
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | (carry ? 1u : 0u);
}

// The last flag writer's PF/AF/ZF/SF/OF escape with CF forced by stc (1) or clc (0); edi and eax are
// saved and restored. CF = 1 ("jae" taken) unless the deadline [0x0066d814] is already past the
// value at [esi+0x29c] on the paths that check it.
static inline void checkDeadline(gp4::Regs* r, const Unknown_0042a9cf_Subject* subject) {
    const int32_t stamp = subject->Unknown29C;
    if (stamp == 0) {
        finish(r, testFlags32(0u), true);
        return;
    }
    const uint32_t flags = addFlags32(0u - static_cast<uint32_t>(stamp), GP4_GLOBAL(uint32_t, 0x0066d814));
    finish(r, flags, (flags & 0x80u) == 0u);
}

// FUNCTION: GP4 0x0042a9cf
void __cdecl FUN_0042a9cf(gp4::Regs* r) {
    const Unknown_0042a9cf_Subject* subject =
        reinterpret_cast<const Unknown_0042a9cf_Subject*>(static_cast<uintptr_t>(r->esi));

    const uint32_t skip = subject->Flags8D & 2u;
    if (skip != 0u) {
        finish(r, testFlags8(skip), true);
        return;
    }
    const uint32_t active = subject->Flags1CE & 0x10u;
    if (active == 0u) {
        finish(r, testFlags8(active), false);
        return;
    }
    const Unknown_0042a9cf_Owner* owner = subject->Unknown1C;
    if ((subject->FlagsCE & 0x80u) == 0u) {
        const uint32_t blocked = owner->Flags12 & 4u;
        if (blocked != 0u) {
            finish(r, testFlags8(blocked), true);
            return;
        }
        checkDeadline(r, subject);
        return;
    }
    if ((owner->Flags51 & 0x20u) != 0u) {
        const uint32_t bit80 = owner->Flags51 & 0x80u;
        if (bit80 == 0u) {
            finish(r, testFlags8(bit80), true);
            return;
        }
    }
    checkDeadline(r, subject);
}
GP4_IMPL(0x0042a9cf, FUN_0042a9cf, "regs(esi:ptr[0x2a4]:layout=rec_0042a9cf) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
