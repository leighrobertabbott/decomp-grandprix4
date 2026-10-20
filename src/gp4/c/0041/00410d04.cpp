#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410d04
// (v2) Resets the car record whose pointer sits in 0x0066d820 (null: nothing happens, flags of `or esi,esi`).
// Clears +0x6f, clears bit 2 of +0xe5, sets bit 4 of +0xce, clears bit 1 of +0x7b, copies the control
// block at 0x007ab548 into the record (0x00410dd2, ebx = block), raises 0x00604418 to 0xff while
// 0x00604419 is zero, runs the one-shot reset 0x0040f6d0 and drops 0x00604418 back to zero.
// Leaves esi = the record, ebx = 0x007ab548 and the flags of the reset routine.
void __cdecl FUN_00410d04(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x0066d820);
    r->esi = record;
    if (record == 0u) {
        // or esi, esi: CF = OF = AF = 0, ZF and PF set for zero, SF clear.
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
    GP4_FIELD(uint8_t, car, 0x6f) = 0;
    GP4_FIELD(uint8_t, car, 0xe5) &= 0xfb;
    GP4_FIELD(uint8_t, car, 0xce) |= 0x10;
    GP4_FIELD(uint8_t, car, 0x7b) &= 0xfd;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = 0x007ab548u;
    gp4::call_regs(0x00410dd2, c);

    if (GP4_GLOBAL(uint8_t, 0x00604419) == 0u) {
        GP4_GLOBAL(uint8_t, 0x00604418) = 0xff;
    }
    gp4::call_regs(0x0040f6d0, c);
    GP4_GLOBAL(uint8_t, 0x00604418) = 0;

    r->ebx = 0x007ab548u;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00410d04, FUN_00410d04, "regs() -> (ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0041104f+flag_007a97a0+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab548+carst_0040f6d0+carf_00410d04")
