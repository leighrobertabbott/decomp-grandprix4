#include <gp4/gp4.h>

struct Unknown_0043e7bd_Record {
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

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the byte operation `left - right`.
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Walks forward through the 0x160-byte record ring (wrapping at DAT_00866ecc / DAT_00891500 /
// DAT_00866ef8) lowering al and the byte at +0x6b of each visited record until that byte minus al
// is non-negative or exactly -1. al and dl hold the last level / difference; ecx and edi are
// restored. The flags are those of the final "sub dl, al" (or "cmp dl, 0xff" on the -1 exit).
// FUNCTION: GP4 0x0043e7bd
void __cdecl FUN_0043e7bd(gp4::Regs* r) {
    uint32_t level = r->eax & 0xffu;
    uint32_t position = r->edi;
    uint32_t difference;
    uint32_t flags;

    for (;;) {
        position += 0x160u;
        if (position >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (position < GP4_GLOBAL(uint32_t, 0x00891500)) {
                position -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (position >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                position = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }

        Unknown_0043e7bd_Record* record = reinterpret_cast<Unknown_0043e7bd_Record*>(static_cast<uintptr_t>(position));
        const uint32_t current = record->Unknown6B;
        difference = (current - level) & 0xffu;
        if ((difference & 0x80u) == 0u) {
            flags = subFlags8(current, level);
            break;
        }
        if (difference == 0xffu) {
            flags = subFlags8(difference, 0xffu);
            break;
        }
        level = (level - 1u) & 0xffu;
        record->Unknown6B = static_cast<uint8_t>(level);
    }

    r->eax = (r->eax & 0xffffff00u) | level;
    r->edx = (r->edx & 0xffffff00u) | difference;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043e7bd, FUN_0043e7bd, "regs(eax:u32, edx:u32, edi:rec[0x8943a0,0x160,0..29]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
