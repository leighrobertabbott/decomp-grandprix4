#include <gp4/gp4.h>
#include <gp4/flags.h>

// push eax / pop eax wrapper: when no slot of the 22-slot table (0x00b1b8d8) is in use with a kind other than 1
// (0x00541cdb with eax = 1 counts them), the selected menu item index from 0x0054107b is stored in the byte
// 0x0081e4f8 and the byte 0x0081e4fa is set to 0xff. Every register except eax is what the callees leave
// (eax is popped); the flags are those of `or eax, eax` or of the last callee.
// FUNCTION: GP4 0x00541059
void __cdecl FUN_00541059(gp4::Regs* r) {
    const uint32_t saved_eax = r->eax;
    gp4::Regs c = *r;
    c.eax = 1u;
    gp4::call_regs(0x00541cdb, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax == 0u) {
        gp4::call_regs(0x0054107b, c);
        GP4_GLOBAL(uint8_t, 0x0081e4f8) = static_cast<uint8_t>(c.eax);
        GP4_GLOBAL(uint8_t, 0x0081e4fa) = 0xffu;
    }
    *r = c;
    r->eax = saved_eax;
}
GP4_IMPL(0x00541059, FUN_00541059, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+full_00541a18+kind_00541cdb+menu_0084300a+items_0053dc57+sel_0053ccc7")
