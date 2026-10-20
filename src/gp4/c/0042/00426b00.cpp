#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `left - right` (cmp).
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Carry-flag predicate: with DAT_007ad894 == 9, 0x0042c0c4 (esi = car) reports CF = 1 and edi lies inside
// the window [DAT_00866f00, [DAT_00866ed0 + 0x44] - 0x840). eax is saved and restored (push/pop) and
// 0x0042c0c4 keeps every register; CF is the answer (stc / clc), the other arithmetic flags are those of the
// last comparison executed.
// FUNCTION: GP4 0x00426b00
void __cdecl FUN_00426b00(gp4::Regs* r) {
    const uint32_t ptr = r->edi;
    bool inside = false;

    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x007ad894);
    uint32_t flags = cmpFlags(mode, 9u);
    if (mode == 9u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eflags = (call.eflags & ~0x8d5u) | flags;
        gp4::call_regs(0x0042c0c4, call);
        flags = call.eflags & 0x8d5u;
        if ((flags & 1u) != 0u) {
            const uint32_t low = GP4_GLOBAL(uint32_t, 0x00866f00);
            flags = cmpFlags(ptr, low);
            if (ptr >= low) {
                uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866ed0)));
                const uint32_t high = GP4_FIELD(uint32_t, segment, 0x44) - 0x840u;
                flags = cmpFlags(ptr, high);
                if (ptr < high) {
                    inside = true;
                }
            }
        }
    }

    flags &= ~1u;
    if (inside) {
        flags |= 1u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426b00, FUN_00426b00, "regs(edi:rec[0x8943a0,0x160,0..26], esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+rel_0042c0c4+state_0043c677+pit_0040d9e6")
