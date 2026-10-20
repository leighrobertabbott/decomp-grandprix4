#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x005297ba with the code 0x2e (the table at 0x7ad8bc instead of 0x7ad8e8 in 0x00503230):
// reads the value of the record's cell through 0x00539944 and hands (0x2e, value) to the stdcall
// routine 0x00503230 inside pushal/popal pairs, so only its flags remain.
// FUNCTION: GP4 0x005297cf
void __cdecl FUN_005297cf(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi)), 0xe);
    gp4::call_regs(0x00539944, c);

    const uint32_t args[2] = {0x2eu, c.eax};
    gp4::Regs d = c;
    gp4::call_regs_stdcall(0x00503230, d, args);
    r->eflags = gp4::flags::merge(r->eflags, d.eflags);
}
GP4_IMPL(0x005297cf, FUN_005297cf, "regs(esi:ptr[0x40]:layout=rec_005297ba) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
