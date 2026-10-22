#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the text with id 0x4bb (0x0054d1a2, esi saved around it), then takes the byte at [esi] (lodsb, esi + 1)
// and prints it as an unformatted number with 0x0054d969 (eax = the byte, bh = 0), skips three more bytes
// (esi = esi + 4 in all) and advances the line counter with 0x0052f1f0. Registers and flags come back from that
// last callee. Assumes DF clear for the lodsb.
// FUNCTION: GP4 0x0052f14c
void __cdecl FUN_0052f14c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.esi = 0x000004bbu;
    gp4::call_regs(0x0054d1a2, c);

    const uint32_t source = r->esi;
    c.esi = source + 1u;
    c.eax = GP4_FIELD(uint8_t, source, 0);
    c.ebx = c.ebx & 0xffff00ffu;
    gp4::call_regs(0x0054d969, c);

    c.esi = source + 4u;
    gp4::call_regs(0x0052f1f0, c);
    *r = c;
}
GP4_IMPL(0x0052f14c, FUN_0052f14c, "regs(ebx:u32, ecx:u32, esi:ptr[0x40]:u8[0..0x30]) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846+textout_0054d211+log_004fd730+lim_0052f1f0")
