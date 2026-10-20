#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script opcode handler: steps esi past the opcode byte, reads the big-endian word that follows
// (bytes swapped by `xchg al, ah` into eax, upper half zero), passes it to the stdcall routine 0x005462cb
// (x position scaled by [0x00b1bf40] / 0x50 into the dword 0x00831560) with the live registers, and
// leaves esi two bytes further. The callee preserves eax/ebx/edx; the flags are those of the final
// `add esi, 2`.
// FUNCTION: GP4 0x0054607c
void __cdecl FUN_0054607c(gp4::Regs* r) {
    const uint32_t cursor = r->esi + 1u;
    const uint32_t word = GP4_GLOBAL(uint16_t, cursor);
    const uint32_t swapped = ((word & 0xffu) << 8) | (word >> 8);

    gp4::Regs c = *r;
    c.esi = cursor;
    c.eax = swapped;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(r->esi, r->eflags));
    const uint32_t args[1] = {swapped};
    gp4::call_regs_stdcall(0x005462cb, c, args);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = cursor + 2u;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(cursor, 2u));
}
GP4_IMPL(0x0054607c, FUN_0054607c, "regs(esi:ptr[0x40]:bytes) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
