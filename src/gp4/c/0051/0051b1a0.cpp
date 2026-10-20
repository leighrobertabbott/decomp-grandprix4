#include <gp4/gp4.h>
#include <gp4/flags.h>

// Palette conversion loop: runs the RGB -> hue/saturation/value converter 0x0051b180 over 0x100
// entries, esi (8-bit RGB triples) advancing by 3 and edi (16-bit results) by 6 each time. ecx is
// saved and restored; esi/edi end 0x300 / 0x600 bytes further and the flags are those of the last
// `add edi, 6`.
// FUNCTION: GP4 0x0051b1a0
void __cdecl FUN_0051b1a0(gp4::Regs* r) {
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
        gp4::call_regs(0x0051b180, c);
        esi += 3u;
        arith = gp4::flags::add32(edi, 6u);
        edi += 6u;
    }
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051b1a0, FUN_0051b1a0, "regs(esi:ptr[0x308]:u8{0,0,1,2,0x40,0x80,0xff,3,7,0x20}, edi:ptr[0x608]:bytes) -> (esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
