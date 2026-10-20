#include <gp4/gp4.h>
#include <gp4/flags.h>

// `push 0 ; call 0x005064f0 ; ret`: the stdcall input poller 0x005064f0 (hand-built COM
// DirectInput device reads, parameter 0 = no channel mask) is called with the live registers; the
// callee keeps ebx/esi/edi/ebp and leaves eax/ecx/edx and the flags of its last flag writer.
// FUNCTION: GP4 0x0051d801
void __cdecl FUN_0051d801(gp4::Regs* r) {
    const uint32_t args[1] = {0u};
    gp4::call_regs_stdcall(0x005064f0, *r, args);
}
GP4_IMPL(0x0051d801, FUN_0051d801, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
