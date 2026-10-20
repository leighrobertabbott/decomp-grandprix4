#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu slider redraw, sign-extending variant (sibling of 0x0053a805, which loads with 0x00539944;
// this one loads the item's value cell with 0x005399ab). pushal/popal wrapper: every register comes
// back unchanged, only the flags of the last flag writer and the memory effects of the callees
// remain. Unless bit 4 of the item byte +0x1f is set, the item's words +4 (ebx), +6 (eax), +8 (ebp)
// and +0xa (ecx) are inserted into the sorted list by 0x00549db9; the empty routine 0x00531b37 runs
// inside a nested pushal/popal; the store routine 0x00531af4 is fed 0x3df1 (bit 5 of +0x1f) or
// 0x1194 and the empty routine 0x00531b2e runs. With bit 3 clear the empty routine 0x0054d709 runs,
// the value of the cell [esi + 0xe] is loaded by 0x005399ab and mapped by 0x0053a978; with bit 3
// set 0x3df1 goes through 0x00531af4 and 0x00531b2e runs again.
// FUNCTION: GP4 0x0053a8f9
void __cdecl FUN_0053a8f9(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint32_t bits = GP4_FIELD(uint8_t, record, 0x1f);
    uint32_t arith = gp4::flags::logic8(bits & 0x10u);
    if ((bits & 0x10u) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    const uint32_t word4 = GP4_FIELD(uint16_t, record, 4);
    const uint32_t word6 = GP4_FIELD(uint16_t, record, 6);
    const uint32_t word8 = GP4_FIELD(uint16_t, record, 8);
    const uint32_t wordA = GP4_FIELD(uint16_t, record, 0xa);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = word4;
    c.eax = word6;
    c.ebp = word8;
    c.ecx = wordA;
    gp4::call_regs(0x00549db9, c);

    // popal brings the registers back; the flags (`add ecx, 3`) are overwritten by the next store.
    gp4::cdecl_<0x00531b37, void>();

    c.eax = (bits & 0x20u) == 0u ? 0x1194u : 0x3df1u;
    gp4::call_regs(0x00531af4, c);
    arith = c.eflags;
    gp4::cdecl_<0x00531b2e, void>();

    c.eax = word6;
    if ((bits & 0x08u) == 0u) {
        gp4::cdecl_<0x0054d709, void>();
        c.ebx = GP4_FIELD(uint32_t, record, 0xe);
        gp4::call_regs(0x005399ab, c);
        gp4::call_regs(0x0053a978, c);
        arith = c.eflags;
    } else {
        c.eax = 0x3df1u;
        gp4::call_regs(0x00531af4, c);
        arith = c.eflags;
        gp4::cdecl_<0x00531b2e, void>();
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053a8f9, FUN_0053a8f9, "regs(esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+list_00549db9")
