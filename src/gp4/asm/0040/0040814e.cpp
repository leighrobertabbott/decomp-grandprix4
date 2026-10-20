#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040814e
// Carry set when dword [esi+0x1d4] is nonzero and ([0x0066d814] - that value) is negative.
// eax is preserved (push/pop); the other flags come from the last flag writer.
void __cdecl FUN_0040814e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t stamp = GP4_FIELD(uint32_t, record, 0x1d4);
    uint32_t flags;
    if (stamp == 0u) {
        // test dword, -1 on zero: PF|ZF, then clc
        flags = 0x44u;
    } else {
        // neg eax / add eax, [0x66d814]: the flags are those of the add
        flags = addFlags(0u - stamp, GP4_GLOBAL(uint32_t, 0x0066d814));
        // jns -> clc, otherwise stc
        flags = (flags & ~1u) | ((flags & 0x80u) != 0u ? 1u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040814e, FUN_0040814e, "regs(esi:ptr[0x1dc]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
