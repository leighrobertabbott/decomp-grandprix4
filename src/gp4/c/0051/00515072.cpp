#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-entry refresh (esi = the 0x22e-byte item record, edi = the readout buffer): loads the colour bytes of
// table row 2 into 0x0088fd94.. (0x00514def), prints the byte [0x007c2e96] (al; the upper bits of eax come
// from 0x00514def) as two digits at edi through 0x00510e71, then clears bit 3 of record byte +0x88 and sets
// 0x00954169 to 0xff. Registers come from the printing routine (esi is read back from it); the flags are
// those of the final `and byte [esi+0x88], 0xf7`.
// FUNCTION: GP4 0x00515072
void __cdecl FUN_00515072(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.eax = 2u;
    gp4::call_regs(0x00514def, c);

    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007c2e96);
    gp4::call_regs(0x00510e71, c);

    uint8_t* flags_byte = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi)) + 0x88;
    const uint32_t cleared = *flags_byte & 0xf7u;
    *flags_byte = static_cast<uint8_t>(cleared);
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(cleared));
    *r = c;
}
GP4_IMPL(0x00515072, FUN_00515072, "regs(esi:ptr[0x22e]:bytes, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187")
