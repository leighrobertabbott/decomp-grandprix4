#include <gp4/gp4.h>
#include <gp4/flags.h>

// Periodic frame registration. When the byte [0x0082fd5e] is nonzero and the tick [0x0081e5bc] has
// reached the due time [0x0082fd74], the due time becomes tick + 0xb4 and the frame object at
// 0x0081ce7c is registered through 0x00528140. eax is pushed and restored; the other registers and
// the flags are what the callee (or the last compare) leaves.
// FUNCTION: GP4 0x0053e992
void __cdecl FUN_0053e992(gp4::Regs* r) {
    const uint32_t eax = r->eax;

    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x0082fd5e);
    uint32_t f = gp4::flags::sub8(enabled, 0u);
    if (enabled != 0u) {
        const uint32_t now = GP4_GLOBAL(uint32_t, 0x0081e5bc);
        const uint32_t due = GP4_GLOBAL(uint32_t, 0x0082fd74);
        f = gp4::flags::sub32(now, due);
        if (now >= due) {
            GP4_GLOBAL(uint32_t, 0x0082fd74) = now + 0xb4u;
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eax = 0x0081ce7cu;
            gp4::call_regs(0x00528140, c);
            *r = c;
            r->eax = eax;
            return;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0053e992, FUN_0053e992, "regs(eax:u32) -> (ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_0052848b+frames_008174f8+frobj_0081ce7c")
