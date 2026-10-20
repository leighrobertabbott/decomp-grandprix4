#include <gp4/gp4.h>
#include <gp4/flags.h>

// Splits two signed bytes of the 8-entry tables at 0x006f3cec and 0x006f3cf4 (index = eax & 7) through the
// fixed-point divide 0x0055936f (edi record divisor): the first quotient's low byte goes to [0x006f3cd4 +
// index]; when the second source byte is not zero its quotient is stored with bit 0 cleared at
// [0x006f3ce4 + index] and cl minus half of it (signed byte shift) at [0x006f3cdc + index]. esi is
// preserved; eax is the divisor of the last divide, ebx the last quotient with its low byte replaced by
// the last byte step; the flags are those of `test bl, 0xff` when the second source is zero, else of the
// `sub cl, bl`.
// FUNCTION: GP4 0x0055933e
void __cdecl FUN_0055933e(gp4::Regs* r) {
    const uint32_t base = 0x006f3cd4u + (r->eax & 7u);
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.ebx = (c.ebx & 0xffffff00u) | GP4_FIELD(uint8_t, base, 0x18);
    gp4::call_regs(0x0055936f, c);
    GP4_FIELD(uint8_t, base, 0) = static_cast<uint8_t>(c.ebx);

    uint32_t second = GP4_FIELD(uint8_t, base, 0x20);
    c.ebx = (c.ebx & 0xffffff00u) | second;
    uint32_t arith = gp4::flags::logic8(second);
    if (second != 0u) {
        gp4::call_regs(0x0055936f, c);
        const uint32_t even = c.ebx & 0xfeu;
        GP4_FIELD(uint8_t, base, 0x10) = static_cast<uint8_t>(even);
        const uint32_t half = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(even)) >> 1) & 0xffu;
        const uint32_t cl = c.ecx & 0xffu;
        arith = gp4::flags::sub8(cl, half);
        const uint32_t reduced = (cl - half) & 0xffu;
        GP4_FIELD(uint8_t, base, 8) = static_cast<uint8_t>(reduced);
        c.ebx = (c.ebx & 0xffffff00u) | half;
        c.ecx = (c.ecx & 0xffffff00u) | reduced;
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055933e, FUN_0055933e, "regs(eax:u32, ebx:u32, ecx:u32, edi:ptr[0x6a]:u8{=0,0,0,1,0x7f,0x80,0xff}) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pan_0055933e")
