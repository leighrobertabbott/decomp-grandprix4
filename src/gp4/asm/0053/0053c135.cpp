#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-item blit step (pushal wrapper). From the item record esi it loads four zero-extended words into
// registers: ebx = word +4, eax = word +6, ebp = word +8, ecx = word +0xa. It inserts that entry into the sorted
// draw list (0x00549db9), runs the two empty routines 0x0054a1d7 and 0x0054d70a, sets ebx = (ebx + ebp) >> 1
// and esi = 0x00823100 (the drawn string buffer), calls the empty routine 0x0054d707 and then the text output
// 0x0054d211. popal restores every register; the flags are those the text output leaves.
// FUNCTION: GP4 0x0053c135
void __cdecl FUN_0053c135(gp4::Regs* r) {
    const uint32_t item = r->esi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = GP4_FIELD(uint16_t, item, 4);
    c.eax = GP4_FIELD(uint16_t, item, 6);
    c.ebp = GP4_FIELD(uint16_t, item, 8);
    c.ecx = GP4_FIELD(uint16_t, item, 0x0a);
    gp4::call_regs(0x00549db9, c);
    gp4::call_regs(0x0054a1d7, c);
    gp4::call_regs(0x0054d70a, c);

    const uint32_t sum = c.ebx + c.ebp;
    const uint32_t half = sum >> 1;
    uint32_t arith = gp4::flags::logic32(half);
    if ((sum & 1u) != 0u) {
        arith |= gp4::flags::CF;
    }
    if ((sum & 0x80000000u) != 0u) {
        arith |= gp4::flags::OF;
    }
    c.ebx = half;
    c.esi = 0x00823100u;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0054d707, c);
    gp4::call_regs(0x0054d211, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053c135, FUN_0053c135, "regs(esi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+textout_0054d211")
