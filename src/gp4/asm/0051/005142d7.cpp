#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sends one character (in al) to the readout writer 0x0051f3bd; every other register stays as the last
// callee left it.
static void put_char(gp4::Regs& c, uint32_t ch) {
    c.eax = (c.eax & 0xffffff00u) | (ch & 0xffu);
    gp4::call_regs(0x0051f3bd, c);
}

// Signed time readout: the value in eax (|value| <= 9999, anything else only leaves the flags of the range
// compare and eax = |value|) is written as a sign ('-' or '+'), a space and the packed BCD of the magnitude
// (0x0051fd6f, stored at 0x88fde0) as "d.ddd": the thousands digit (high nibble of the byte 0x88fde1), '.',
// the hundreds digit, then the tens and units digits (bytes 0x88fde0). Registers and flags are those of the
// last 0x0051f3bd call.
// FUNCTION: GP4 0x005142d7
void __cdecl FUN_005142d7(gp4::Regs* r) {
    gp4::Regs c = *r;
    int32_t value = static_cast<int32_t>(r->eax);
    uint32_t sign = 0x2bu;
    if (value < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
        sign = 0x2du;
    }
    if (value > 0x270f) {
        c.eax = static_cast<uint32_t>(value);
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(static_cast<uint32_t>(value), 0x270fu));
        *r = c;
        return;
    }

    const uint32_t kept = static_cast<uint32_t>(value);
    c.eax = (kept & 0xffffff00u) | sign;
    gp4::call_regs(0x0051f3bd, c);
    put_char(c, 0x20u);
    c.eax = kept;

    gp4::call_regs(0x0051fd6f, c);
    GP4_GLOBAL(uint32_t, 0x0088fde0) = c.eax;
    put_char(c, ((GP4_GLOBAL(uint8_t, 0x0088fde1) >> 4) & 0xfu) + 0x30u);
    put_char(c, 0x2eu);
    put_char(c, (GP4_GLOBAL(uint8_t, 0x0088fde1) & 0xfu) + 0x30u);
    put_char(c, ((GP4_GLOBAL(uint8_t, 0x0088fde0) >> 4) & 0xfu) + 0x30u);
    put_char(c, (GP4_GLOBAL(uint8_t, 0x0088fde0) & 0xfu) + 0x30u);
    *r = c;
}
GP4_IMPL(0x005142d7, FUN_005142d7, "regs(eax:i32[-10100..10100], edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
