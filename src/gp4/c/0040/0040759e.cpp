#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040759e
// Runs 0x004074db (edi = the record it reads; returns eax/edx/ecx), stores the three results in
// DAT_006278b0 / DAT_006278b4 / DAT_006278bc and, when byte DAT_0062794a is nonzero, hands
// eax = DAT_006278b4 and edx = DAT_006278b0 to 0x0041ec08 and returns what it leaves. Otherwise
// returns eax = DAT_006278b4 with the flags of "cmp byte [0x0062794a], 0".
void __cdecl FUN_0040759e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x004074db, c);
    GP4_GLOBAL(uint32_t, 0x006278b0) = c.eax;
    GP4_GLOBAL(uint32_t, 0x006278b4) = c.edx;
    GP4_GLOBAL(uint32_t, 0x006278bc) = c.ecx;
    c.eax = GP4_GLOBAL(uint32_t, 0x006278b4);

    const uint32_t value = GP4_GLOBAL(uint8_t, 0x0062794a);
    // cmp byte [0x0062794a], 0: CF = AF = OF = 0
    const uint32_t flags = parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
    c.eflags = (c.eflags & ~0x8d5u) | flags;
    if (value != 0u) {
        c.edx = GP4_GLOBAL(uint32_t, 0x006278b0);
        gp4::call_regs(0x0041ec08, c);
    }
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0040759e, FUN_0040759e, "regs(edi:rec[0x8943a0,0x160,0..11]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040759e")
