#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the object slots 0x16..0x3f. A slot whose mark byte [0x00866e47 + slot] is not 0xff and that has an
// active body (0x0043ae70, eax = slot, returns the 0x41c-byte car body or 0) is checked by 0x0043ae98
// (eax = [0x00679d20], ecx = slot, edx = 0xce40: CF set when the slot's object is within 0xce40 of the
// position); then 0x00552bea allocates a record for the slot (ecx = slot, esi = the body) and, when it
// found a free one (CF clear), the mark byte is set to 0xff. The registers carry from one slot to the next;
// the flags are those of the final `cmp ecx, 0x40`.
// FUNCTION: GP4 0x005538a0
void __cdecl FUN_005538a0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint8_t* const marks = GP4_ARRAY(uint8_t, 0x00866e47);

    c.ecx = 0x16u;
    do {
        const uint8_t mark = marks[c.ecx];
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(mark, 0xffu));
        if (mark != 0xffu) {
            c.eax = c.ecx;
            gp4::call_regs(0x0043ae70, c);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
            if (c.eax != 0u) {
                c.esi = c.eax;
                c.eax = GP4_GLOBAL(uint32_t, 0x00679d20);
                c.edx = 0xce40u;
                gp4::call_regs(0x0043ae98, c);
                if ((c.eflags & gp4::CF) != 0u) {
                    gp4::call_regs(0x00552bea, c);
                    if ((c.eflags & gp4::CF) == 0u) {
                        marks[c.ecx] = 0xffu;
                    }
                }
            }
        }
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.ecx, c.eflags));
        c.ecx += 1u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 0x40u));
    } while (c.ecx != 0x40u);

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005538a0, FUN_005538a0, "regs(eax:u32, edx:u32, esi:u32) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_0063bd7c+tbl_00854460")
