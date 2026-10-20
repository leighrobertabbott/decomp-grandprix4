#include <gp4/gp4.h>

// pushal/popal wrapper that sets up the entry table of a bounding pass: clears the three point
// coordinates DAT_0062a064/68/6c, copies the record table pointer DAT_0062a044 to DAT_0062a058 and
// the entry count DAT_0062a04c (at least 1) to DAT_0062a05c, clears that many bytes of the threshold
// table (0x00422632), then runs the 0x004226ae sweep 23 times with the mask DAT_0062a054 = 1, 2, 4,
// ... (doubled after every sweep), and finishes with 0x004225c0 and 0x00422604. Every register is
// restored; the flags are those 0x00422604 leaves.
// FUNCTION: GP4 0x00422642
void __cdecl FUN_00422642(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0062a064) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a068) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a06c) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a058) = GP4_GLOBAL(uint32_t, 0x0062a044);

    int32_t entries = GP4_GLOBAL(int32_t, 0x0062a04c);
    if (entries < 1) {
        entries = 1;
    }
    GP4_GLOBAL(int32_t, 0x0062a05c) = entries;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = static_cast<uint32_t>(entries);
    gp4::call_regs(0x00422632, c);

    GP4_GLOBAL(uint32_t, 0x0062a054) = 1;
    for (uint32_t pass = 0x17; pass != 0; --pass) {
        c.ecx = pass;
        gp4::call_regs(0x004226ae, c);
        GP4_GLOBAL(uint32_t, 0x0062a054) = GP4_GLOBAL(uint32_t, 0x0062a054) << 1;
    }
    gp4::call_regs(0x004225c0, c);
    gp4::call_regs(0x00422604, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00422642, FUN_00422642, "regs(ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042458b+state_0042257b+state_004225c0+list_0062a048")
