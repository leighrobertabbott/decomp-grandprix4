#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tail of 0x00540159 (entered with the flags of `or eax, eax`): with ZF clear, eax points at an object
// whose byte +0x14 equal to 1 selects the mask ebx = 0xff. The mask then goes (eax = ebx) to the static
// item chain walker 0x0053e4db with esi = 0x0084869c; every register and the flags come back from it.
// FUNCTION: GP4 0x00540188
void __cdecl FUN_00540188(gp4::Regs* r) {
    gp4::Regs c = *r;
    if ((r->eflags & gp4::ZF) == 0u) {
        const uint8_t kind = GP4_FIELD(uint8_t, r->eax, 0x14);
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(kind, 1u));
        if (kind == 1u) {
            c.ebx = 0xffu;
        }
    }
    c.esi = 0x0084869cu;
    c.eax = c.ebx;
    gp4::call_regs(0x0053e4db, c);
    *r = c;
}
GP4_IMPL(0x00540188, FUN_00540188, "regs(eax:ptr[0x20]:u8{0,1,1,2}, ebx:u32, zf:u32[0..1]) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=layout=flags_0084869c")
