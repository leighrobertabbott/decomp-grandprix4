#include <gp4/gp4.h>
#include <gp4/flags.h>

// Latches a code (eax) with 0x0054aca4, stores/re-reads the packet with 0x0054b8fe, clears the byte
// 0x007aafd9 and compares the byte 0x007aafd3 with 0xff. Registers come back as 0x0054b8fe leaves them;
// the arithmetic flags are those of the final `cmp byte [0x7aafd3], 0xff`.
// FUNCTION: GP4 0x0054ac8b
void __cdecl FUN_0054ac8b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054aca4, c);
    gp4::call_regs(0x0054b8fe, c);
    GP4_GLOBAL(uint8_t, 0x007aafd9) = 0;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007aafd3), 0xffu));
    *r = c;
}
GP4_IMPL(0x0054ac8b, FUN_0054ac8b, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0054b978+mode_007aafd3")
