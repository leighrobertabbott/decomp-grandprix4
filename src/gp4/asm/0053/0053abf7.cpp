#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item draw dispatcher on the item at esi. When bit 4 of its flag byte +0x1f is clear the whole job is
// the register preserving item painter 0x0053ac2b. Otherwise (inside pushal/popal) the word 0x6318 is stored
// by 0x00531af4, the item's unsigned words +4 (ebx), +6 (eax), +8 (ebp) and +0xa (ecx) are loaded, the empty
// routine 0x00531b2e runs and the sorted-list insert 0x00549db9 queues the rectangle. Every register comes
// back either way; the flags are those the last callee leaves.
// FUNCTION: GP4 0x0053abf7
void __cdecl FUN_0053abf7(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint32_t bits = GP4_FIELD(uint8_t, record, 0x1f);

    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bits & 0x10u));
    if ((bits & 0x10u) == 0u) {
        gp4::call_regs(0x0053ac2b, c);
    } else {
        c.eax = 0x6318u;
        gp4::call_regs(0x00531af4, c);
        c.ebx = GP4_FIELD(uint16_t, record, 4);
        c.eax = GP4_FIELD(uint16_t, record, 6);
        c.ebp = GP4_FIELD(uint16_t, record, 8);
        c.ecx = GP4_FIELD(uint16_t, record, 0xa);
        gp4::cdecl_<0x00531b2e, void>();
        gp4::call_regs(0x00549db9, c);
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053abf7, FUN_0053abf7, "regs(esi:ptr[0x40]:u8{0,0x10,0x20,0x30,0x80,0x90,0xa0,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+sel_0053aab9")
