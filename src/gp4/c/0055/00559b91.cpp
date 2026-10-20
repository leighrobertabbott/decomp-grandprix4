#include <gp4/gp4.h>
#include <gp4/flags.h>

// edi = [0x00866f08] + 0x2520 (27 records of 0x160 bytes on); ax = 0x300 & word [edi + 0x2c]. Then
// 0x00559bab ORs ax into five words of that record; eax keeps its upper half, edi stays the record
// address, the flags are those of the callee's last `or word [edi + 0x40], ax`.
// FUNCTION: GP4 0x00559b91
void __cdecl FUN_00559b91(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00866f08) + 0x2520u;
    const uint32_t bits = 0x300u & GP4_FIELD(uint16_t, record, 0x2c);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = record;
    c.eax = (r->eax & 0xffff0000u) | bits;
    gp4::call_regs(0x00559bab, c);

    r->eax = c.eax;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00559b91, FUN_00559b91, "regs(eax:u32) -> (eax, edi, cf, pf, af, zf, sf, of); globals=layout=state_0040cb38+records_008943a0+segbits_00559b91")
