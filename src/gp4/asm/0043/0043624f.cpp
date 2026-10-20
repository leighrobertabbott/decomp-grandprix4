#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043624f
// pushal ... popal routine. esi = car record: +0 / +4 are the position (x, z) and +0x1c the current
// track-segment record. Clears [0x627944] and [0x627947], sets [0x627945] = 0xff, runs the segment
// locator 0x00406dbb (eax = x, edx = z, edi = segment) and stores the resulting segment back to
// +0x1c. Bit 0 of +0x1ce becomes [0x627946] != 0 (car on a pit-lane entry); the result of 0x00407811
// (called with the locator's eax) goes to [0x006278c4]. Then 0x004322a6, 0x00409349 and 0x00409416
// refresh the record from the new segment, each seeing the registers the previous one left. All
// registers are restored; the flags left are those 0x00409416 leaves.
void __cdecl FUN_0043624f(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint8_t, 0x00627945) = 0xff;
    GP4_GLOBAL(uint8_t, 0x00627944) = 0;
    GP4_GLOBAL(uint8_t, 0x00627947) = 0;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.edi = GP4_FIELD(uint32_t, car, 0x1c);
    s.eax = GP4_FIELD(uint32_t, car, 0);
    s.edx = GP4_FIELD(uint32_t, car, 4);
    gp4::call_regs(0x00406dbb, s);

    GP4_FIELD(uint32_t, car, 0x1c) = s.edi;
    if (GP4_GLOBAL(uint8_t, 0x00627946) != 0u) {
        GP4_FIELD(uint8_t, car, 0x1ce) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x1ce) | 0x01u);
    } else {
        GP4_FIELD(uint8_t, car, 0x1ce) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x1ce) & 0xfeu);
    }

    gp4::call_regs(0x00407811, s);
    GP4_GLOBAL(uint32_t, 0x006278c4) = s.eax;

    gp4::call_regs(0x004322a6, s);
    gp4::call_regs(0x00409349, s);
    gp4::call_regs(0x00409416, s);

    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x0043624f, FUN_0043624f, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00412b88) -> (cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=trackgeo_008943a0+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0")
