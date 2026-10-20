#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041e910
// edi = one of the 0x160-byte records at 0x008943a0 (the first, the one 0x2c0 bytes before the
// pointer at 0x00866ecc, or any other); esi = the factor given to 0x0041eaa7. The record kind picks a
// 16-byte row of four dwords at 0x006279fc; each row dword plus the 0x0041eaa7 result (taken for the
// first record) is stored to 0x00637ff0..0x00637ffc, and 0x00627858/5c/60 are set to 0x40000.
// edx = the 0x0041eaa7 result; eax and the flags are those of the last "add".
void __cdecl FUN_0041e910(gp4::Regs* r) {
    const uint32_t record = r->edi;
    const uint32_t factor = r->esi;

    uint32_t row = 1;
    if (record == 0x008943a0u) {
        row = 2;
    } else if (record == GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x2c0u) {
        row = 0;
    }

    gp4::Regs sub = *r;
    sub.st_in = 0;
    sub.st_out = 0;
    sub.edi = 0x008943a0u;
    sub.eax = 0;
    sub.edx = factor;
    gp4::call_regs(0x0041eaa7, sub);
    const uint32_t offset = sub.eax;

    const uint32_t* table = GP4_ARRAY(uint32_t, 0x006279fc) + row * 4;
    uint32_t* out = GP4_ARRAY(uint32_t, 0x00637ff0);
    out[0] = table[0] + offset;
    out[1] = table[1] + offset;
    out[2] = table[2] + offset;
    const uint32_t flags = additionFlags32(table[3], offset);
    out[3] = table[3] + offset;

    GP4_GLOBAL(uint32_t, 0x00627858) = 0x40000;
    GP4_GLOBAL(uint32_t, 0x0062785c) = 0x40000;
    GP4_GLOBAL(uint32_t, 0x00627860) = 0x40000;

    r->eax = out[3];
    r->edx = offset;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041e910, FUN_0041e910, "regs(edi:rec[0x8943a0,0x160,0..25], esi:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=records_008943a0+multiply_0040ce7b+dvar_0041eacf")
