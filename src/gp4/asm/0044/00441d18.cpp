#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `neg ax` (16-bit): result = 0 - ax, CF = (ax != 0), OF = (ax == 0x8000).
static inline uint32_t neg16Flags(uint32_t value, uint32_t result) {
    return (value != 0u ? 0x01u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (value == 0x8000u ? 0x800u : 0u);
}

struct Unknown_Pair {
    uint8_t Unknown00[0x28];
    uint8_t Low28;
    uint8_t High29;
    uint8_t Unknown2A[0x1e];
    uint8_t Flags48;
};

struct Unknown_Out {
    uint8_t Flags00;
    uint8_t Unknown01;
    int16_t Value02;
};

// Turns the smaller of the two bytes +0x28 / +0x29 of the record in edi into a signed word stored at
// [ebx + 2]: level * 8 (or 0x30 and the flag bit 0x80 of [ebx] when a clamp bit of +0x48 is set or the
// product reaches 0x50) + 0x130, floored at 0x64, negated when +0x28 < +0x29 (dx = 0xffff then, else 0).
// ax / dx are the low halves of eax / edx on return; the flags are those of `neg ax` for the negated case
// and of `test dx, 0xffff` (dx = 0) otherwise.
// FUNCTION: GP4 0x00441d18
void __cdecl FUN_00441d18(gp4::Regs* r) {
    const Unknown_Pair* record = reinterpret_cast<const Unknown_Pair*>(static_cast<uintptr_t>(r->edi));
    Unknown_Out* out = reinterpret_cast<Unknown_Out*>(static_cast<uintptr_t>(r->ebx));

    uint32_t dx = 0xffffu;
    uint32_t level;
    bool clamp;
    if (record->Low28 < record->High29) {
        level = record->Low28;
        clamp = (record->Flags48 & 0x20u) != 0u;
    } else {
        dx = 0u;
        level = record->High29;
        clamp = (record->Flags48 & 0x10u) != 0u;
    }

    uint32_t ax = level;
    if (!clamp) {
        ax = (ax << 3) & 0xffffu;
        clamp = static_cast<int16_t>(ax) >= 0x50;
    }
    if (clamp) {
        ax = 0x30u;
        out->Flags00 |= 0x80u;
    }

    ax = (ax + 0x130u) & 0xffffu;
    if (static_cast<int16_t>(ax) < 0x64) {
        ax = 0x64u;
    }

    uint32_t flags = 0x44u;                       // test dx, 0xffff with dx = 0: ZF, PF
    if (dx != 0u) {
        const uint32_t negated = (0u - ax) & 0xffffu;
        flags = neg16Flags(ax, negated);
        ax = negated;
    }
    out->Value02 = static_cast<int16_t>(ax);

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->edx = (r->edx & 0xffff0000u) | dx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441d18, FUN_00441d18, "regs(eax:u32, edx:u32, ebx:ptr[0x10]:bytes, edi:ptr[0x50]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
