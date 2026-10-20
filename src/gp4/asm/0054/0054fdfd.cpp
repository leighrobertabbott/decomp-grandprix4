#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi = scale-box record (the one 0x0054fe11 reads through [0x0083e69e]): publishes it in
// [0x0083e69e], calls 0x00503940 (stdcall, one argument: the record) between pushal/popal, then runs
// 0x0054fe11. The stdcall callee's registers and flags are discarded by popal / overwritten by
// 0x0054fe11, which restores every register itself and leaves the arithmetic flags.
// FUNCTION: GP4 0x0054fdfd
void __cdecl FUN_0054fdfd(gp4::Regs* r) {
    const uint32_t box = r->esi;
    GP4_GLOBAL(uint32_t, 0x0083e69e) = box;
    gp4::stdcall_<0x00503940, int>(box);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054fe11, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054fdfd, FUN_0054fdfd, "regs(esi:ptr[0x10]:layout=box_0054fe11) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")
