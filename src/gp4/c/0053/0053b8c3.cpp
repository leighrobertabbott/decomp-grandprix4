#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: stores the current local date word in [0x0082459c] (0x0052acfa), formats that day count
// as date text into the buffer 0x00823100 (0x0052ae53; edi ends up after the text) and writes a NUL at [edi].
// popal restores every register; then 0x0053c135 (esi = the menu item record) draws the buffer with them,
// leaving its flags, and the byte 0x0082fd5f is set to 0xff.
// FUNCTION: GP4 0x0053b8c3
void __cdecl FUN_0053b8c3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0052acfa, c);
    c.eax = GP4_GLOBAL(uint16_t, 0x0082459c);
    gp4::call_regs(0x0052ae53, c);
    GP4_FIELD(uint8_t, c.edi, 0) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
    gp4::call_regs(0x0053c135, *r);
    GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
}
GP4_IMPL(0x0053b8c3, FUN_0053b8c3, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x20]:bytes, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+textsel_00528c98+script_00545c84+list_00549db9+pool_00549db9+textout_0054d211")
