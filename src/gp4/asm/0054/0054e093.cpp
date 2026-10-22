#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the text of the edit record at edi (+0 start of the text, +8 cursor) through the character routine
// 0x0054d3a1 (al = glyph code; the row/column words are 0x00833cc9 / 0x00833cc5). The row starts at
// [0x00838c1c] and each line starts at the column [0x00838c18]; a control character (below 0x20) ends the
// line (routine 0x00531b3d is an empty `ret` called with the line's registers), a NUL ends the text. When the
// scan reaches the cursor and the byte [0x00838c29] is 0xff, the cursor is drawn: colour 0x7fff (0x7c00 when
// bit 2 of [0x0081e5bc] is set) through 0x0054cc96, the glyph '|' (0x7c), then the colour 0x7fff again.
// Every line start resets the byte [0x00833f74] to 0xff; every non-final line adds 0x12 to the row.
// Registers on exit: esi = one past the terminating NUL, ebx = the line's column, ecx = the row + 0x12,
// ebp = [0x00838c20], edx = [0x00838c18], eax = the last colour/row source with al = 0, edi unchanged. The flags
// are those of the final `or al, al` (ZF = PF = 1).
// FUNCTION: GP4 0x0054e093
void __cdecl FUN_0054e093(gp4::Regs* r) {
    const uint32_t record = r->edi;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.esi = GP4_FIELD(uint32_t, record, 0);
    c.eax = GP4_GLOBAL(uint32_t, 0x00838c1c);
    GP4_GLOBAL(uint32_t, 0x00833cc9) = c.eax;

    uint32_t ch;
    for (;;) {
        GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
        c.edx = GP4_GLOBAL(uint32_t, 0x00838c18);
        GP4_GLOBAL(uint32_t, 0x00833cc5) = c.edx;

        for (;;) {
            if (c.esi == GP4_FIELD(uint32_t, record, 8) && GP4_GLOBAL(uint8_t, 0x00838c29) == 0xffu) {
                c.eax = 0x7fffu;
                if ((GP4_GLOBAL(uint32_t, 0x0081e5bc) & 4u) != 0u) {
                    c.eax = 0x7c00u;
                }
                gp4::call_regs(0x0054cc96, c);
                c.eax = (c.eax & 0xffffff00u) | 0x7cu;
                gp4::call_regs(0x0054d3a1, c);
                c.eax = 0x7fffu;
                gp4::call_regs(0x0054cc96, c);
            }
            ch = GP4_GLOBAL(uint8_t, c.esi);
            c.esi = c.esi + 1u;
            c.eax = (c.eax & 0xffffff00u) | ch;
            if (ch < 0x20u) {
                break;
            }
            gp4::call_regs(0x0054d3a1, c);
        }

        c.ebp = GP4_GLOBAL(uint32_t, 0x00838c20);
        c.ebx = GP4_GLOBAL(uint32_t, 0x00833cc5);
        c.ecx = GP4_GLOBAL(uint32_t, 0x00833cc9) + 0x12u;
        gp4::Regs line = c;
        line.eax = GP4_GLOBAL(uint32_t, 0x00833cc9);
        gp4::call_regs(0x00531b3d, line);

        if (ch == 0u) {
            break;
        }
        GP4_GLOBAL(uint32_t, 0x00833cc9) = GP4_GLOBAL(uint32_t, 0x00833cc9) + 0x12u;
    }

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(0u));
    *r = c;
}
GP4_IMPL(0x0054e093, FUN_0054e093, "regs(ebx:u32, ecx:u32, edi:ptr[0x10]:u32{=0xa20000,0xa20004,0xa20010,0xa20020,0xa20030}) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=text_0054e12a+log_004fd730")
