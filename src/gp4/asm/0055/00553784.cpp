#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the record routine 0x0055278a with eax = 0xb when bit 6 of the byte at esi + 0x87 is set, else
// eax = 1. Every register and the flags come back from it.
// FUNCTION: GP4 0x00553784
void __cdecl FUN_00553784(gp4::Regs* r) {
    const uint32_t flagByte = GP4_FIELD(uint8_t, r->esi, 0x87);
    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flagByte & 0x40u));
    c.eax = (flagByte & 0x40u) != 0u ? 0xbu : 1u;
    gp4::call_regs(0x0055278a, c);
    *r = c;
}
GP4_IMPL(0x00553784, FUN_00553784, "regs(esi:ptr[0xe0]:layout=fxowner_0055278a) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=tbl_00854460+trackgeo_008943a0+tbl_00868598+state_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+w6_008943a0+fx_00866248")
