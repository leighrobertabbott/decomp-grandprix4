#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-edit record at edi: +0 start of the text, +4 end of the text, +8 cursor. Scans the text from
// the start counting lines and columns: dh = line (starts at 0, +1 after each control character below
// 0x20 other than NUL), dl = column (bytes read since the line began, the terminator included). The
// scan stops when esi reaches the cursor (esi = cursor), or at a NUL byte (esi = the record's end
// pointer +4). al ends as the last byte read; eax's other bits and edx's upper half are kept. Both
// exits leave ZF = PF = 1 (cmp of equal values / `or al, al` with al = 0), everything else clear.
// FUNCTION: GP4 0x0054e045
void __cdecl FUN_0054e045(gp4::Regs* r) {
    const uint32_t record = r->edi;
    uint32_t esi = GP4_FIELD(uint32_t, record, 0);
    uint32_t line = 0xffu;
    uint32_t column = 0u;
    uint32_t al = r->eax & 0xffu;

    for (;;) {
        line = (line + 1u) & 0xffu;
        column = 0u;
        bool newline = false;
        for (;;) {
            if (esi == GP4_FIELD(uint32_t, record, 8)) {
                r->esi = esi;
                r->edx = (r->edx & 0xffff0000u) | (line << 8) | column;
                r->eax = (r->eax & 0xffffff00u) | al;
                r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(esi, esi));
                return;
            }
            al = GP4_GLOBAL(uint8_t, esi);
            esi = esi + 1u;
            column = (column + 1u) & 0xffu;
            if (al >= 0x20u) {
                continue;
            }
            newline = (al != 0u);
            break;
        }
        if (!newline) {
            break;
        }
    }

    r->esi = GP4_FIELD(uint32_t, record, 4);
    r->edx = (r->edx & 0xffff0000u) | (line << 8) | column;
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(al));
}
GP4_IMPL(0x0054e045, FUN_0054e045, "regs(eax:u32, edx:u32, edi:ptr[0x10]:u32{=0xa20000,0xa20004,0xa20010,0xa20020,0xa20030}) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=text_0054e12a")
