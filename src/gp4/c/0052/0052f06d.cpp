#include <gp4/gp4.h>
#include <gp4/flags.h>

// Prints "<space><number><space><string 0xcb>" at the text cursor: the character 0x20 goes through the
// text-capture wrapper 0x0054d3a1, then the dword at 0x00826d21 is printed by the number formatter
// 0x0054d969 (bh = 0: no width, no sign), then another 0x20, then the table string 0xcb is drawn by
// 0x0054d1a2 (esi = 0xcb is turned into the string pointer, which the callee leaves in esi).
// al is set to 0x20 before both character calls, so eax ends as the dword's upper 24 bits with al = 0x20;
// bh is cleared; the registers and flags otherwise come back from the last callee.
// FUNCTION: GP4 0x0052f06d
void __cdecl FUN_0052f06d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = (c.eax & 0xffffff00u) | 0x20u;
    gp4::call_regs(0x0054d3a1, c);

    c.eax = GP4_GLOBAL(uint32_t, 0x00826d21);
    c.ebx = c.ebx & 0xffff00ffu;
    gp4::call_regs(0x0054d969, c);

    c.eax = (c.eax & 0xffffff00u) | 0x20u;
    gp4::call_regs(0x0054d3a1, c);

    c.esi = 0xcbu;
    gp4::call_regs(0x0054d1a2, c);

    *r = c;
}
GP4_IMPL(0x0052f06d, FUN_0052f06d, "regs(eax:u32, ebx:u32) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+log_004fd730+textout_0054d211")
