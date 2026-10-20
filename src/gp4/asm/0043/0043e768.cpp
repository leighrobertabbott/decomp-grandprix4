#include <gp4/gp4.h>

struct UnknownRecord_0043e768 {
    uint8_t Unknown00[0x6b];
    uint8_t Unknown6B;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 8-bit `sub left, right` whose result is `result`.
static inline uint32_t sub8Flags(uint32_t left, uint32_t right, uint32_t result) {
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result) & 0x80u) != 0u) ? 0x800u : 0u);
}

// Walks backwards through the 0x160-byte records of the ring starting at edi: while the byte at
// +0x6b minus al is negative (and not -1), al is decremented and stored there, then edi moves to the
// previous record (wrapping by DAT_00866ec4 below DAT_00891504 when edi was below DAT_00891500,
// otherwise clamped at DAT_00891500). al and the low byte of edx are left modified; ecx and edi are
// preserved; the flags are those of the exit test.
// FUNCTION: GP4 0x0043e768
void __cdecl FUN_0043e768(gp4::Regs* r) {
    uint32_t record = r->edi;
    uint32_t al = r->eax & 0xffu;
    uint32_t dl;
    uint32_t flags;

    for (;;) {
        UnknownRecord_0043e768* entry = (UnknownRecord_0043e768*)(uintptr_t)record;
        const uint32_t current = entry->Unknown6B;
        dl = (current - al) & 0xffu;
        if ((dl & 0x80u) == 0u) {
            flags = sub8Flags(current, al, dl);
            break;
        }
        if (dl == 0xffu) {
            // cmp dl, 0xff with dl == 0xff: ZF | PF
            flags = 0x44u;
            break;
        }
        al = (al - 1u) & 0xffu;
        entry->Unknown6B = static_cast<uint8_t>(al);
        if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
            record -= 0x160u;
            if (record < GP4_GLOBAL(uint32_t, 0x00891504)) {
                record += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            record -= 0x160u;
            if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
                record = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        }
    }

    r->eax = (r->eax & 0xffffff00u) | al;
    r->edx = (r->edx & 0xffffff00u) | dl;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043e768, FUN_0043e768, "regs(eax:u32, edx:u32, edi:rec[0x8943a0,0x160,0..29]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
