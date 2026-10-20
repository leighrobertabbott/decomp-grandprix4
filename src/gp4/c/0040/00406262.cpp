#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `sub left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of `test byte ptr [..], 8` (CF = OF = AF = 0).
static inline uint32_t testBit3Flags(uint32_t byteValue) {
    const uint32_t result = byteValue & 8u;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u);
}

struct Unknown_00406262 {
    uint8_t Unknown00[0x92];
    uint8_t Flags92;
};

static inline bool recordFlagged(uint32_t record) {
    return (reinterpret_cast<const Unknown_00406262*>(static_cast<uintptr_t>(record))->Flags92 & 8u) != 0u;
}

// Decides on which side of the record at edi the point (DAT_00623e30, DAT_00623e38) lies (carry from
// 0x00405e19) and returns the neighbouring record in edx: the next one when the point is on the far
// side of it, the previous one otherwise. Records whose byte +0x92 has bit 3 set are stepped with
// the wrapping helpers 0x00406051 / 0x00406077. eax is the last dot-product sum; the flags are those
// of the last flag-writing instruction. ebx, ecx, edi and ebp are saved and restored.
// FUNCTION: GP4 0x00406262
void __cdecl FUN_00406262(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.ecx = GP4_GLOBAL(uint32_t, 0x00623e30);
    s.ebx = GP4_GLOBAL(uint32_t, 0x00623e38);

    gp4::call_regs(0x00405e19, s);
    bool behind = (s.eflags & gp4::CF) != 0u;
    if (!behind) {
        s.edi += 0x160u;
        const bool wrapped = recordFlagged(s.edi);
        s.eflags = (s.eflags & ~0x8d5u) | testBit3Flags(wrapped ? 8u : 0u);
        if (wrapped) {
            gp4::call_regs(0x00406051, s);
        }
        gp4::call_regs(0x00405e19, s);
        behind = (s.eflags & gp4::CF) != 0u;
    }
    if (behind) {
        if (recordFlagged(s.edi)) {
            s.eflags = (s.eflags & ~0x8d5u) | testBit3Flags(8u);
            gp4::call_regs(0x00406077, s);
        } else {
            s.eflags = (s.eflags & ~0x8d5u) | subFlags32(s.edi, 0x160u);
            s.edi -= 0x160u;
        }
    }

    r->eax = s.eax;
    r->edx = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00406262, FUN_00406262, "regs(edi:rec[0x8943a0,0x160,0..11]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
