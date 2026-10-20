#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item draw handler on the item at esi (pushal/popal wrapper): the frame record 0x2c4 of the table
// 0x8174f8 (loaded by 0x00527eee; CF set when the frame table is not loaded) gives the origin, the item's
// words +4/+6 the size; the rectangle is queued by the sorted-list insert 0x00549db9. Unless bit 4 of the flag
// byte +0x1f is set, frame 0x2c4 (0x2c5 when +0x1e has 0x40 and +0x1f has 0x80) is drawn by 0x0052800e at the
// item's (x, y); then the item's value (read from the cell at +0xe by 0x00539944, plus the dword +0x26) picks
// a second frame drawn one pixel to the lower right. popal restores every register; the flags are those of
// the last flag writer on the path taken.
// FUNCTION: GP4 0x0053adde
void __cdecl FUN_0053adde(gp4::Regs* r) {
    const uint32_t item = r->esi;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0x2c4u;
    c.edx = c.eax;
    gp4::call_regs(0x00527eee, c);
    if ((c.eflags & gp4::CF) == 0u) {
        c.ebx = GP4_FIELD(uint16_t, item, 4);
        c.eax = GP4_FIELD(uint16_t, item, 6);
        c.ebp = c.ebp + c.ebx - 1u;
        c.ecx = c.ecx + c.eax - 1u;
        gp4::call_regs(0x00549db9, c);
        const uint8_t bits = GP4_FIELD(uint8_t, item, 0x1f);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bits & 0x10u));
        if ((bits & 0x10u) == 0u) {
            if ((GP4_FIELD(uint8_t, item, 0x1e) & 0x40u) != 0u) {
                if ((GP4_FIELD(uint8_t, item, 0x1f) & 0x80u) != 0u) {
                    c.edx = c.edx + 1u;
                }
            }
            gp4::call_regs(0x0052800e, c);
            const uint32_t saved_eax = c.eax;
            const uint32_t saved_ebx = c.ebx;
            c.ebx = GP4_FIELD(uint32_t, item, 0xe);
            gp4::call_regs(0x00539944, c);
            c.eax = c.eax + GP4_FIELD(uint32_t, item, 0x26);
            c.edx = c.eax;
            c.ebx = saved_ebx + 1u;
            c.eax = saved_eax + 1u;
            gp4::call_regs(0x0052800e, c);
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053adde, FUN_0053adde, "regs(esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+list_00549db9+pool_00549db9")
