#include <gp4/gp4.h>
#include <gp4/flags.h>

// Palette conversion loop: runs the 8-bit to 6-bit RGB triple converter 0x0051b123 over 0x100 entries,
// advancing esi (source) and edi (destination) by 3 bytes each time. ecx is saved and restored; esi/edi
// end 0x300 bytes further and the flags are those of the last `add edi, 3`.
// FUNCTION: GP4 0x0051b1e4
void __cdecl FUN_0051b1e4(gp4::Regs* r) {
    uint32_t esi = r->esi;
    uint32_t edi = r->edi;
    uint32_t arith = 0u;
    for (uint32_t n = 0x100u; n != 0u; --n) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.esi = esi;
        c.edi = edi;
        gp4::call_regs(0x0051b123, c);
        esi += 3u;
        arith = gp4::flags::add32(edi, 3u);
        edi += 3u;
    }
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051b1e4, FUN_0051b1e4, "regs(esi:ptr[0x308]:bytes, edi:ptr[0x308]:bytes) -> (esi, edi, cf, pf, af, zf, sf, of)")
