#include <gp4/gp4.h>
#include <gp4/flags.h>

// Queues the menu item esi's rectangle for drawing and hands the frame cell on. pushal/popal wrapper: every
// register comes back unchanged, only the flags of the last flag writer and the callees' memory effects remain.
// The frame number is the item's value (read from the cell [esi+0xe] at the width of its type by 0x00539944)
// plus the dword +0x26; 0x00527eee loads that frame's rectangle (ebx, eax, ebp, ecx from the 0x18-byte table
// 0x008174f8) and returns with CF set when the table is not loaded, which ends the routine with its flags.
// Otherwise the item's own words +4 (ebx) and +6 (eax) are loaded, ebp and ecx become the frame's values plus
// x - 1 and y - 1, and 0x00549db9 inserts the node into the sorted pool list. Items with bit 4 of the byte
// +0x1f go to the empty routine 0x00531b3d (flags of that test); the others run the frame-cell hand-off
// 0x0052800e with edx = the frame number, one more when bit 6 of +0x1e and bit 7 of +0x1f are both set.
// FUNCTION: GP4 0x0053ae38
void __cdecl FUN_0053ae38(gp4::Regs* r) {
    const uint32_t record = r->esi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.ebx = GP4_FIELD(uint32_t, record, 0xe);
    gp4::call_regs(0x00539944, c);

    const uint32_t frame = c.eax + GP4_FIELD(uint32_t, record, 0x26);
    c.eax = frame;
    c.edx = frame;
    gp4::call_regs(0x00527eee, c);
    if ((c.eflags & gp4::CF) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }

    const uint32_t x = GP4_FIELD(uint16_t, record, 4);
    const uint32_t y = GP4_FIELD(uint16_t, record, 6);
    c.ebx = x;
    c.eax = y;
    c.ebp = c.ebp + x - 1u;
    c.ecx = c.ecx + y - 1u;
    gp4::call_regs(0x00549db9, c);

    const uint32_t bits = GP4_FIELD(uint8_t, record, 0x1f);
    if ((bits & 0x10u) != 0u) {
        gp4::cdecl_<0x00531b3d, void>();
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bits & 0x10u));
        return;
    }

    if ((GP4_FIELD(uint8_t, record, 0x1e) & 0x40u) != 0u && (bits & 0x80u) != 0u) {
        c.edx = c.edx + 1u;
    }
    gp4::call_regs(0x0052800e, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053ae38, FUN_0053ae38, "regs(esi:u32{=0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224,0x83ef4a,0x84050c,0x847696,0x84facb}, edi:ptr[8]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+mitems_0053ddbb+sel_007ab544+cell_0053ae38+list_00549db9+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba")
