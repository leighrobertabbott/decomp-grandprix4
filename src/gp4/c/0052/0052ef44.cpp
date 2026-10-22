#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the date/time-like line of a text screen through the text output 0x0054d3a1 (character in al) and the
// number formatter 0x0054d969 (value in eax, bh = mode bits, bl = pad, cl = width). The first part prints the
// three fields (byte [0x826d1c] + 1, byte [0x826d1d] + 1, word [0x826d1a]) with no width, separated by the
// character [0x829750]; the order of the three depends on the byte [0x7ab17c] (0, 1 or anything else, clamped
// to 2): (c, d, w), (d, c, w) or (w, c, d). Then a blank (0x20), byte [0x826d1e] + 1 as two zero-padded
// digits (bh 0x40, bl 0x30, cl 2), a colon (0x3a) and byte [0x826d1f] as two zero-padded digits. Registers
// thread through the callees (they keep every register), the flags are those the last formatter leaves.
static void __declspec(noinline) put_number(gp4::Regs& c, uint32_t value) {
    c.eax = value;
    c.ebx = c.ebx & 0xffff00ffu;
    gp4::call_regs(0x0054d969, c);
}

static void __declspec(noinline) put_sep(gp4::Regs& c) {
    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00829750);
    gp4::call_regs(0x0054d3a1, c);
}

static void __declspec(noinline) put_padded(gp4::Regs& c, uint32_t value) {
    c.eax = value;
    c.ebx = (c.ebx & 0xffff0000u) | 0x4030u;
    c.ecx = (c.ecx & 0xffffff00u) | 2u;
    gp4::call_regs(0x0054d969, c);
}

// FUNCTION: GP4 0x0052ef44
void __cdecl FUN_0052ef44(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t mode = GP4_GLOBAL(uint8_t, 0x007ab17c);
    if (mode >= 3u) {
        mode = 2u;
    }

    if (mode == 0u) {
        put_number(c, GP4_GLOBAL(uint8_t, 0x00826d1c) + 1u);
        put_sep(c);
        put_number(c, GP4_GLOBAL(uint8_t, 0x00826d1d) + 1u);
        put_sep(c);
        put_number(c, GP4_GLOBAL(uint16_t, 0x00826d1a));
    } else if (mode == 1u) {
        put_number(c, GP4_GLOBAL(uint8_t, 0x00826d1d) + 1u);
        put_sep(c);
        put_number(c, GP4_GLOBAL(uint8_t, 0x00826d1c) + 1u);
        put_sep(c);
        put_number(c, GP4_GLOBAL(uint16_t, 0x00826d1a));
    } else {
        put_number(c, GP4_GLOBAL(uint16_t, 0x00826d1a));
        put_sep(c);
        put_number(c, GP4_GLOBAL(uint8_t, 0x00826d1c) + 1u);
        put_sep(c);
        put_number(c, GP4_GLOBAL(uint8_t, 0x00826d1d) + 1u);
    }

    c.eax = (c.eax & 0xffffff00u) | 0x20u;
    gp4::call_regs(0x0054d3a1, c);
    put_padded(c, GP4_GLOBAL(uint8_t, 0x00826d1e) + 1u);
    c.eax = (c.eax & 0xffffff00u) | 0x3au;
    gp4::call_regs(0x0054d3a1, c);
    put_padded(c, GP4_GLOBAL(uint8_t, 0x00826d1f));

    *r = c;
}
GP4_IMPL(0x0052ef44, FUN_0052ef44, "regs(ebx:u32, ecx:u32) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_004fd730")
