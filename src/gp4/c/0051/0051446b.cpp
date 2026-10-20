#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sends one character (in al) to the readout writer 0x0051f3bd; every other register stays as the last
// callee left it.
static void put_char(gp4::Regs& c, uint32_t ch) {
    c.eax = (c.eax & 0xffffff00u) | (ch & 0xffu);
    gp4::call_regs(0x0051f3bd, c);
}

// Cursor advance after a separator: when the selector dword [0x0088fe14] is -1 the row byte 0x88fd94 is
// incremented (CF kept), otherwise the column byte 0x88fd96 grows by 5. Returns the flags of the compare and
// of the byte update.
static uint32_t step_cursor(uint32_t flags) {
    const uint32_t selector = GP4_GLOBAL(uint32_t, 0x0088fe14);
    flags = gp4::flags::merge(flags, gp4::flags::sub32(selector, 0xffffffffu));
    if (selector == 0xffffffffu) {
        const uint32_t row = GP4_GLOBAL(uint8_t, 0x0088fd94);
        GP4_GLOBAL(uint8_t, 0x0088fd94) = static_cast<uint8_t>(row + 1u);
        const uint32_t inc = (gp4::flags::add8(row, 1u) & ~gp4::flags::CF) | (flags & gp4::flags::CF);
        return gp4::flags::merge(flags, inc);
    }
    const uint32_t col = GP4_GLOBAL(uint8_t, 0x0088fd96);
    GP4_GLOBAL(uint8_t, 0x0088fd96) = static_cast<uint8_t>(col + 5u);
    return gp4::flags::merge(flags, gp4::flags::add8(col, 5u));
}

// Writes six '-' placeholders to the readout (the "no time" display), advancing the cursor after the first
// and after the third. Registers and flags are those the last 0x0051f3bd call leaves.
// FUNCTION: GP4 0x0051446b
void __cdecl FUN_0051446b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    put_char(c, 0x2du);
    c.eflags = step_cursor(c.eflags);
    put_char(c, 0x2du);
    put_char(c, 0x2du);
    c.eflags = step_cursor(c.eflags);
    put_char(c, 0x2du);
    put_char(c, 0x2du);
    put_char(c, 0x2du);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051446b, FUN_0051446b, "regs(eax:u32, edi:ptr[0x100]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
