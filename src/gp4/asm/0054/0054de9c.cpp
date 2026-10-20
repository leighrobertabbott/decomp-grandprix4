#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-edit record at edi (+0 text start, +4 end pointer, +8 cursor). Steps the cursor forward over line feeds
// (0x0a); if it reaches the end pointer nothing else happens (the flags are those of the failed `cmp cursor,
// end` and edx holds the cursor reached). Otherwise the character at the new position is deleted (0x0054e13f
// with eax = 1, edx = the position), the cursor is stored and the text is re-wrapped (0x0054ded0); registers and
// flags come back from that call.
// FUNCTION: GP4 0x0054de9c
void __cdecl FUN_0054de9c(gp4::Regs* r) {
    const uint32_t record = r->edi;
    uint32_t cursor = GP4_FIELD(uint32_t, record, 8);
    uint32_t arith;
    for (;;) {
        const uint32_t end = GP4_FIELD(uint32_t, record, 4);
        arith = gp4::flags::sub32(cursor, end);
        if (static_cast<int32_t>(cursor) >= static_cast<int32_t>(end)) {
            r->edx = cursor;
            r->eflags = gp4::flags::merge(r->eflags, arith);
            return;
        }
        const uint8_t ch = GP4_FIELD(uint8_t, cursor, 0);
        arith = gp4::flags::sub8(ch, 0x0au);
        if (ch != 0x0au) {
            break;
        }
        cursor += 1u;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = 1u;
    c.edx = cursor;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0054e13f, c);
    GP4_FIELD(uint32_t, record, 8) = c.edx;
    gp4::call_regs(0x0054ded0, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054de9c, FUN_0054de9c, "regs(edi:ptr[0x10]:layout=rec_0054defe, eax:u32, ebx:u32, esi:u32) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=text_0054defe")
