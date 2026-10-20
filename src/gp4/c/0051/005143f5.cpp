#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sends one character (in al) to the readout writer 0x0051f3bd; every other register stays as the last
// callee left it.
static void put_char(gp4::Regs& c, uint32_t ch) {
    c.eax = (c.eax & 0xffffff00u) | (ch & 0xffu);
    gp4::call_regs(0x0051f3bd, c);
}

// Writes the time in eax (milliseconds) to the readout as "h:mm.tt": 0x0051fd6f converts it to packed BCD
// (stored at [0x0088fde0]); the high and low nibble of byte 2, ':', the low nibble of byte 2 ... are sent as
// digits ('0' + nibble) - byte 2 high nibble, ':', byte 2 low nibble, byte 1 high nibble, '.', byte 1 low
// nibble, byte 0 high nibble, byte 0 low nibble. Registers and flags are those the last 0x0051f3bd call
// leaves.
// FUNCTION: GP4 0x005143f5
void __cdecl FUN_005143f5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0051fd6f, c);
    GP4_GLOBAL(uint32_t, 0x0088fde0) = c.eax;
    put_char(c, ((GP4_GLOBAL(uint8_t, 0x0088fde2) >> 4) & 0xfu) + 0x30u);
    put_char(c, 0x3au);
    put_char(c, (GP4_GLOBAL(uint8_t, 0x0088fde2) & 0xfu) + 0x30u);
    put_char(c, ((GP4_GLOBAL(uint8_t, 0x0088fde1) >> 4) & 0xfu) + 0x30u);
    put_char(c, 0x2eu);
    put_char(c, (GP4_GLOBAL(uint8_t, 0x0088fde1) & 0xfu) + 0x30u);
    put_char(c, ((GP4_GLOBAL(uint8_t, 0x0088fde0) >> 4) & 0xfu) + 0x30u);
    put_char(c, (GP4_GLOBAL(uint8_t, 0x0088fde0) & 0xfu) + 0x30u);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005143f5, FUN_005143f5, "regs(eax:i32[-100..30000000], edi:ptr[0x100]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
