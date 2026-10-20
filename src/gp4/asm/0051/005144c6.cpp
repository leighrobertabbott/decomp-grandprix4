#include <gp4/gp4.h>
#include <gp4/flags.h>

// Entry k of the dword table at 0x00891bd0 (indexed in a helper so the compiler keeps the table base as the
// only global, as in the original).
static __declspec(noinline) uint32_t TableEntry(uint32_t k) {
    return GP4_ARRAY(uint32_t, 0x00891bd0)[k];
}

// Sends one character (in al) to the readout writer 0x0051f3bd; every other register stays as the last
// callee left it.
static void put_char(gp4::Regs& c, uint32_t ch) {
    c.eax = (c.eax & 0xffffff00u) | (ch & 0xffu);
    gp4::call_regs(0x0051f3bd, c);
}

// Advances the glyph column bytes 0x88fd94 / 0x88fd96 without a readout (the INT_MIN and out-of-range cases).
static uint32_t skip_columns() {
    const uint32_t row = GP4_GLOBAL(uint8_t, 0x0088fd94);
    GP4_GLOBAL(uint8_t, 0x0088fd94) = static_cast<uint8_t>(row + 5u);
    const uint32_t col = GP4_GLOBAL(uint8_t, 0x0088fd96);
    GP4_GLOBAL(uint8_t, 0x0088fd96) = static_cast<uint8_t>(col + 1u);
    return gp4::flags::add8(col, 1u);
}

// Readout of the value in eax. Negative: a magnitude of 1..9 (the low byte after `neg al`) is written as one
// digit followed by the unit string of table entry 0x2e (magnitude 1) or 0x2d, the column bytes advancing by 2
// (before the string) and 6 (after it) and the row byte 0x88fd94 by -1; INT_MIN and larger magnitudes only
// advance the columns (al is negated first except for INT_MIN). Non-negative: the packed BCD of eax
// (0x0051fe26, stored at 0x88fde0) is written as "m:ss.t" - the high nibble and low nibble of byte 0x88fde2,
// ':', then the nibbles of byte 0x88fde1 around a '.'. esi is saved and restored; the flags are those of the
// last callee, or of the final `add byte`.
// FUNCTION: GP4 0x005144c6
void __cdecl FUN_005144c6(gp4::Regs* r) {
    gp4::Regs c = *r;
    const uint32_t value = r->eax;
    if (static_cast<int32_t>(value) < 0) {
        if (value == 0x80000000u) {
            r->eflags = gp4::flags::merge(r->eflags, skip_columns());
            return;
        }
        const uint32_t digit = (0u - value) & 0xffu;
        c.eax = (value & 0xffffff00u) | digit;
        if (digit >= 0xau) {
            r->eax = c.eax;
            r->eflags = gp4::flags::merge(r->eflags, skip_columns());
            return;
        }
        c.esi = TableEntry(0x2eu);
        if (digit != 1u) {
            c.esi = TableEntry(0x2du);
        }
        put_char(c, (digit & 0xfu) + 0x30u);
        const uint32_t col = GP4_GLOBAL(uint8_t, 0x0088fd96);
        GP4_GLOBAL(uint8_t, 0x0088fd96) = static_cast<uint8_t>(col + 2u);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add8(col, 2u));
        gp4::call_regs(0x0051f3c3, c);
        c.esi = r->esi;
        const uint32_t row = GP4_GLOBAL(uint8_t, 0x0088fd94);
        GP4_GLOBAL(uint8_t, 0x0088fd94) = static_cast<uint8_t>(row - 1u);
        const uint32_t col2 = GP4_GLOBAL(uint8_t, 0x0088fd96);
        GP4_GLOBAL(uint8_t, 0x0088fd96) = static_cast<uint8_t>(col2 + 6u);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add8(col2, 6u));
        *r = c;
        return;
    }

    gp4::call_regs(0x0051fe26, c);
    GP4_GLOBAL(uint32_t, 0x0088fde0) = c.eax;
    put_char(c, ((GP4_GLOBAL(uint8_t, 0x0088fde2) >> 4) & 0xfu) + 0x30u);
    put_char(c, 0x3au);
    put_char(c, (GP4_GLOBAL(uint8_t, 0x0088fde2) & 0xfu) + 0x30u);
    put_char(c, ((GP4_GLOBAL(uint8_t, 0x0088fde1) >> 4) & 0xfu) + 0x30u);
    put_char(c, 0x2eu);
    put_char(c, (GP4_GLOBAL(uint8_t, 0x0088fde1) & 0xfu) + 0x30u);
    *r = c;
}
GP4_IMPL(0x005144c6, FUN_005144c6, "regs(eax:u32{=0,1,0x7fffffff,0x80000000,0xffffffff,0xfffffffe,0xfffffff7,0xfffffff6,0xfffffff0,0x100,0x12345678,0xffffff00,100,99999,5999999}, edi:ptr[0x100]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329+units_005144c6")
