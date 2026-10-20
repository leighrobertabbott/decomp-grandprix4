#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00432436
void __cdecl FUN_00432436(gp4::Regs* r) {
    // Clears [0x602b58]. If bit 0x80 or 0x40 of byte [esi+0x80] is set, draws two bytes from
    // the generator at 0x401462 and stores ((a + b - 0xff) << 4) + 0x4650 there.
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(uint32_t, 0x00602b58) = 0u;
    const uint8_t mode = body[0x80];
    if ((mode & 0x80u) == 0u && (mode & 0x40u) == 0u) {
        // flags of `test byte [esi+0x80], 0x40` with a zero result
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
    gp4::Regs c;
    c.eax = r->eax;
    c.ebx = r->ebx;
    c.ecx = r->ecx;
    c.edx = r->edx;
    c.esi = r->esi;
    c.edi = r->edi;
    c.ebp = r->ebp;
    c.eflags = r->eflags;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00401462, c);
    const uint32_t first = c.eax & 0xffu;
    c.eax = first;
    gp4::call_regs(0x00401462, c);
    const uint32_t second = c.eax & 0xffu;
    const uint32_t scaled = (second + first - 0xffu) << 4;
    const uint32_t value = scaled + 0x4650u;
    GP4_GLOBAL(uint32_t, 0x00602b58) = value;
    r->eax = value;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = first;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(scaled, 0x4650u);
}
GP4_IMPL(0x00432436, FUN_00432436, "regs(eax:u32, edx:u32, esi:ptr[0x88]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
