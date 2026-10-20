#include <gp4/gp4.h>
#include <gp4/flags.h>

// One step of a fade: while the frame counter [0x007a9780] is nonzero it runs the empty palette
// upload 0x00541e39 and the empty stub 0x005ae990, decrements the counter and either eases the
// 0x300-byte table with 0x00541f51 (counter still nonzero) or finishes: byte 0x007a9792 = 0 and
// byte 0x007a9791 = ~[0x007a9790] (al gets that value). Leaves the registers and flags of the
// last callee / compare.
// FUNCTION: GP4 0x00541f14
void __cdecl FUN_00541f14(gp4::Regs* r) {
    if (GP4_GLOBAL(uint32_t, 0x007a9780) != 0u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x00541e39, c);
        gp4::call_regs(0x005ae990, c);
        const uint32_t count = GP4_GLOBAL(uint32_t, 0x007a9780) - 1u;
        GP4_GLOBAL(uint32_t, 0x007a9780) = count;
        if (count == 0u) {
            GP4_GLOBAL(uint8_t, 0x007a9792) = 0u;
            const uint8_t inverted = static_cast<uint8_t>(~GP4_GLOBAL(uint8_t, 0x007a9790));
            c.eax = (c.eax & 0xffffff00u) | inverted;
            GP4_GLOBAL(uint8_t, 0x007a9791) = inverted;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(count, 0u));
        } else {
            gp4::call_regs(0x00541f51, c);
        }
        *r = c;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
    }
}
GP4_IMPL(0x00541f14, FUN_00541f14, "regs(eax:u32, ebx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00541f51")
