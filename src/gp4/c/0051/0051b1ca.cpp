#include <gp4/gp4.h>
#include <gp4/flags.h>

// Palette conversion loop: runs the hue/saturation/value -> 16-bit RGB converter 0x0051b175 and the
// 16-bit to 8-bit narrowing 0x0051b041 over 0x100 entries; esi (16-bit triples in) advances by 6 and
// edi (8-bit triples out) by 3 each time. ecx is saved and restored; esi/edi end 0x600 / 0x300
// bytes further and the flags are those of the last `add edi, 3`.
// FUNCTION: GP4 0x0051b1ca
void __cdecl FUN_0051b1ca(gp4::Regs* r) {
    uint32_t esi = r->esi;
    uint32_t edi = r->edi;
    uint32_t arith = 0u;
    for (uint32_t n = 0x100u; n != 0u; --n) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.ecx = n;
        c.esi = esi;
        c.edi = edi;
        gp4::call_regs(0x0051b175, c);
        gp4::call_regs(0x0051b041, c);
        esi += 6u;
        arith = gp4::flags::add32(edi, 3u);
        edi += 3u;
    }
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051b1ca, FUN_0051b1ca, "regs(esi:ptr[0x608]:bytes, edi:ptr[0x308]:bytes) -> (esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
