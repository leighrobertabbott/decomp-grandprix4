#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: 0x0052b02b turns the day count in eax into the date (ecx = year, dh = month, dl = day),
// the byte [0x007ab17c] (0, 1, anything else clamped to 2) picks one of the three date formatters
// 0x00545c13 / 0x00545c84 / 0x00545cf7 which write the date text at edi = 0x00823100, and the script
// interpreter 0x00545f09 then walks that text (esi = 0x00823100). popal restores every register; the flags are
// those the interpreter leaves.
// FUNCTION: GP4 0x00545bbf
static __declspec(noinline) void format_date_0(gp4::Regs& c) { gp4::call_regs(0x00545c13, c); }
static __declspec(noinline) void format_date_1(gp4::Regs& c) { gp4::call_regs(0x00545c84, c); }
static __declspec(noinline) void format_date_2(gp4::Regs& c) { gp4::call_regs(0x00545cf7, c); }

void __cdecl FUN_00545bbf(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0052b02b, c);
    c.edi = 0x00823100u;

    uint32_t mode = GP4_GLOBAL(uint8_t, 0x007ab17c);
    if (mode >= 3u) {
        mode = 2u;
    }
    c.eax = 0x00545be8u + (mode << 2);
    if (mode == 0u) {
        format_date_0(c);
    } else if (mode == 1u) {
        format_date_1(c);
    } else {
        format_date_2(c);
    }

    c.esi = 0x00823100u;
    gp4::call_regs(0x00545f09, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00545bbf, FUN_00545bbf, "regs(eax:u32[0..0x20000], ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_00545c84+cursor_00545f09+surf_0054654a")
