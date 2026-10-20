#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043bcd4
// pushal ... popal wrapper: for each of the 22 cars (esi = 0..21) whose flag byte in the table at 0x007c2a48
// has bit 7 clear, 0x0043bb15 (eax = car) selects the car's data pointer 0x00601f24, then 0x0043cb8f runs
// with ebx = that pointer and edx = the car. All registers are restored by popal; the flags are those of
// the final `cmp esi, 0x16` (equal: ZF and PF set).
void __cdecl FUN_0043bcd4(gp4::Regs* r) {
    for (uint32_t car = 0; car != 0x16u; ++car) {
        if (static_cast<int8_t>(GP4_ARRAY(uint8_t, 0x007c2a48)[car]) >= 0) {
            gp4::Regs callee = *r;
            callee.st_in = 0;
            callee.st_out = 0;
            callee.esi = car;
            callee.eax = car;
            gp4::call_regs(0x0043bb15, callee);
            callee.ebx = GP4_GLOBAL(uint32_t, 0x00601f24);
            callee.edx = car;
            gp4::call_regs(0x0043cb8f, callee);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043bcd4, FUN_0043bcd4, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+circ_007adb50+codes_0043ba12+adj_0066d802+codes_007adb54+state_0043bcd4")
