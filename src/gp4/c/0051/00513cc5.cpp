#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the 0x00513112 pass with the byte [0x007c2ead] temporarily cleared: the byte is read into al, saved
// with eax on the stack, zeroed for the call and written back afterwards. eax comes back with al = the saved
// byte (upper 24 bits as the caller left them); every other register and the flags are what 0x00513112 leaves.
// FUNCTION: GP4 0x00513cc5
void __cdecl FUN_00513cc5(gp4::Regs* r) {
    const uint8_t saved = GP4_GLOBAL(uint8_t, 0x007c2ead);
    const uint32_t eax = (r->eax & 0xffffff00u) | saved;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    GP4_GLOBAL(uint8_t, 0x007c2ead) = 0u;
    gp4::call_regs(0x00513112, c);

    c.eax = eax;
    GP4_GLOBAL(uint8_t, 0x007c2ead) = saved;
    *r = c;
}
GP4_IMPL(0x00513cc5, FUN_00513cc5, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
