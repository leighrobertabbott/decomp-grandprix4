#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" on a byte.
static inline uint32_t compareFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of "or reg, reg" / "or reg, imm" on a dword: CF = OF = AF = 0.
static inline uint32_t orFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// When byte DAT_007c2ea9 is 0x80, three random 16-bit draws (0x0043269a, eax = threshold 0x2aaa,
// 0x1000 and 0x800) set bits 2, 0x10 and 0x20 of ebp; the result is stored as a word at +0x64 of the
// record in esi. ebp is the result register (cleared first); eax and the flags are those left by
// the last draw / bit set.
// FUNCTION: GP4 0x004326ca
void __cdecl FUN_004326ca(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->ebp = 0;

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = compareFlags8(mode, 0x80u);
    if (mode == 0x80u) {
        r->eax = 0x2aaau;
        gp4::call_regs(0x0043269a, *r);
        flags = orFlags32(r->eax);
        if (r->eax != 0u) {
            r->ebp |= 2u;
            flags = orFlags32(r->ebp);
        }

        r->eax = 0x1000u;
        gp4::call_regs(0x0043269a, *r);
        flags = orFlags32(r->eax);
        if (r->eax != 0u) {
            r->ebp |= 0x10u;
            flags = orFlags32(r->ebp);
        }

        r->eax = 0x800u;
        gp4::call_regs(0x0043269a, *r);
        flags = orFlags32(r->eax);
        if (r->eax != 0u) {
            r->ebp |= 0x20u;
            flags = orFlags32(r->ebp);
        }
    }

    GP4_FIELD(uint16_t, record, 0x64) = static_cast<uint16_t>(r->ebp);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004326ca, FUN_004326ca, "regs(esi:ptr[0x70]:bytes) -> (eax, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_0040f112")
