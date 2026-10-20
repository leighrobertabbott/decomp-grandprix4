#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parser for one comma-separated line of a text table: esi is the input cursor, edi the output cursor.
// Blanks are skipped (0x005146e8) and each item is dispatched on its first character: ',' is skipped;
// ';' or CR ends the line (0x005146f2 swallows the rest of it); '"' copies a quoted string
// (0x00514734); '+', '-' or a digit ('0'..'9', a signed compare) is parsed as a number and stored as a
// byte (0x005147a3, after `dec esi`); 'D'/'d' runs the unit-suffix check 0x00514705 (after `dec esi`);
// anything else is copied like a string. Registers and flags are those the last callee left.
// A 'D'/'d' that is not followed by "B"/"b" and a blank is re-read forever (0x00514705 leaves esi on
// the 'D'), so the game's text tables never contain one.
// FUNCTION: GP4 0x005147aa
void __cdecl FUN_005147aa(gp4::Regs* r) {
    gp4::Regs c = *r;
    for (;;) {
        gp4::call_regs(0x005146e8, c);
        const int32_t ch = static_cast<int8_t>(c.eax & 0xffu);
        if (ch == 0x2c) {
            continue;
        }
        if (ch == 0x3b || ch == 0x0d) {
            gp4::call_regs(0x005146f2, c);
            break;
        }
        if (ch == 0x22) {
            gp4::call_regs(0x00514734, c);
            continue;
        }
        if (ch == 0x2b || ch == 0x2d || (ch >= 0x30 && ch < 0x3a)) {
            c.esi = c.esi - 1u;
            gp4::call_regs(0x005147a3, c);
            continue;
        }
        if (ch == 0x44 || ch == 0x64) {
            c.esi = c.esi - 1u;
            gp4::call_regs(0x00514705, c);
            continue;
        }
        gp4::call_regs(0x00514734, c);
    }
    *r = c;
}
GP4_IMPL(0x005147aa, FUN_005147aa, "regs(eax:u32, esi:ptr[0x40]:layout=text_005147aa, edi:ptr[0x400]:bytes) -> (eax, esi, edi, cf, pf, af, zf, sf, of)")
