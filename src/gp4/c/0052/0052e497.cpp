#include <gp4/gp4.h>

// `call 0x0052e589 ; mov byte [0x0081e4fb], 0xff ; ret`: rebuilds the file-browser listing through the
// regs routine 0x0052e589, then sets the byte flag at 0x0081e4fb to 0xff (a mov: the flags and
// registers are the callee's).
// FUNCTION: GP4 0x0052e497
void __cdecl FUN_0052e497(gp4::Regs* r) {
    gp4::call_regs(0x0052e589, *r);
    GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
}
GP4_IMPL(0x0052e497, FUN_0052e497, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
