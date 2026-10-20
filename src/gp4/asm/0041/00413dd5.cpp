#include <gp4/gp4.h>

static const uint32_t kArithFlags = 0x8d5u;

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte / word / dword logic result (TEST / AND): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Publishes the callee-visible registers (eax, edx, ebp changed only by the callees) and the flags:
// the arithmetic flags come from `flags` (the last flag writer) with CF forced to `carry` by clc / stc.
static inline void finish(gp4::Regs* r, const gp4::Regs& c, uint32_t flags, uint32_t carry) {
    r->eax = c.eax;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~kArithFlags) | (flags & kArithFlags & ~1u) | carry;
}

// 0x00434d6b / 0x00413d49 / 0x00413d01 each return a verdict in CF and leave the other flags of their last
// compare; the call keeps those flags.
static inline uint32_t callChecked(gp4::Regs& c, uint32_t address) {
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(address, c);
    return c.eflags & kArithFlags;
}

// Decides whether the car record at esi passes the checks of the pit/track entry condition: CF = 1 when it
// does (stc), CF = 0 (clc) at the first failed test. Along the way it clears the deferred-state bits
// (+0x179 bit 0, +0x230 bit 6, +0x203 bit 5) once the timer word at +0x1f0 has run out of [0x66d814].
// The other flags are those of the last test / compare executed.
// FUNCTION: GP4 0x00413dd5
void __cdecl FUN_00413dd5(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    uint32_t flags;

    const uint32_t blocked = GP4_FIELD(uint8_t, body, 0x25f) & 0x80u;
    if (blocked != 0u) {
        finish(r, c, logicFlags8(blocked), 0u);
        return;
    }

    if ((GP4_FIELD(uint8_t, body, 0x179) & 1u) != 0u) {
        const uint32_t deadline = GP4_FIELD(uint32_t, body, 0x1f0);
        if (deadline != 0u) {
            const uint32_t negated = 0u - deadline;
            const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
            if (static_cast<int32_t>(negated + now) < 0) {
                finish(r, c, addFlags32(negated, now), 0u);
                return;
            }
        }
        GP4_FIELD(uint8_t, body, 0x179) = static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x179) & 0xfeu);
        GP4_FIELD(uint8_t, body, 0x230) = static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x230) & 0xbfu);
        GP4_FIELD(uint8_t, body, 0x203) = static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x203) & 0xdfu);
    }

    bool skipTests = (GP4_FIELD(uint8_t, body, 0x26b) & 8u) != 0u;
    if (!skipTests) {
        skipTests = (GP4_FIELD(uint8_t, body, 0x25f) & 2u) != 0u;
    }
    if (!skipTests) {
        const uint32_t busy = GP4_FIELD(uint32_t, body, 0x40) & 0x15280u;
        if (busy != 0u) {
            finish(r, c, logicFlags32(busy), 0u);
            return;
        }

        flags = callChecked(c, 0x00434d6b);
        if ((flags & 1u) != 0u) {
            finish(r, c, flags, 0u);
            return;
        }

        const uint32_t progress = GP4_FIELD(uint32_t, body, 0x1a0);
        if (static_cast<int32_t>(progress) < 0) {
            finish(r, c, compareFlags32(progress, 0u), 0u);
            return;
        }

        const uint32_t marked = GP4_FIELD(uint8_t, body, 0xe5) & 0x80u;
        if (marked != 0u) {
            finish(r, c, logicFlags8(marked), 0u);
            return;
        }

        flags = callChecked(c, 0x00413d49);
        if ((flags & 1u) == 0u) {
            finish(r, c, flags, 0u);
            return;
        }

        const uint32_t word = GP4_FIELD(uint16_t, body, 0x9a);
        if ((word & 0x8000u) != 0u) {
            finish(r, c, logicFlags16(word), 0u);
            return;
        }
    }

    const uint32_t locked = GP4_FIELD(uint8_t, body, 0xcc) & 0x40u;
    if (locked != 0u) {
        finish(r, c, logicFlags8(locked), 0u);
        return;
    }

    if ((GP4_FIELD(uint8_t, body, 0x25f) & 2u) == 0u) {
        const uint32_t armed = GP4_FIELD(uint8_t, body, 0xfe) & 0x20u;
        if (armed == 0u) {
            finish(r, c, logicFlags8(armed), 0u);
            return;
        }
    }

    const uint32_t held = GP4_FIELD(uint8_t, body, 0x7a) & 0x20u;
    if (held != 0u) {
        finish(r, c, logicFlags8(held), 0u);
        return;
    }

    flags = callChecked(c, 0x00413d01);
    if ((flags & 1u) != 0u) {
        finish(r, c, flags, 0u);
        return;
    }
    finish(r, c, flags, 1u);
}
GP4_IMPL(0x00413dd5, FUN_00413dd5, "regs(eax:u32, esi:ptr[0x35c]:layout=rec_00413dd5, edi:ptr[0x6a]:layout=edi_00413d49) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00413dd5+state_00413d01")
