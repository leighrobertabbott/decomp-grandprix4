#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs 0x0054ae7c (its CF is set when the mode byte 0x007aafd3 is below 0xff); when CF is clear it also
// runs 0x0054aff0. It ends on `cmp byte [0x007aafd3], 0xff`, so the flags are those of that compare and
// the registers are whatever the callees left.
// FUNCTION: GP4 0x0054ae68
void __cdecl FUN_0054ae68(gp4::Regs* r) {
    gp4::call_regs(0x0054ae7c, *r);
    if ((r->eflags & 1u) == 0u) {
        gp4::call_regs(0x0054aff0, *r);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007aafd3), 0xffu));
}
GP4_IMPL(0x0054ae68, FUN_0054ae68, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_007aafd3; emu=skip:win32 PeekMessageA")
