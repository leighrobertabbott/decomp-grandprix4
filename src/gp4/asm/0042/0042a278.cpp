#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042a278
// Picks which of two car-record routines to run for the record in esi: 0x0042ba87 when the target
// car (dword +0x124, which also goes to ebx) has bit 6 of +0xfe set and bit 4 clear, or, with bit 6
// clear, from the signed words +0x90 / +0x94 (|difference| >= 0x200 and moving away from zero, or
// +0x94 negative); 0x0042bae4 otherwise. The callee's CF and flags are the result (the stc / clc
// tails only copy its CF). dx keeps |+0x90 - +0x94| or +0x90 on the paths that reach the callee.
void __cdecl FUN_0042a278(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t target = GP4_FIELD(uint32_t, rec, 0x124);
    r->ebx = target;
    const uint8_t targetFlags = GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(target)), 0xfe);

    bool first;
    if ((targetFlags & 0x40u) != 0u) {
        first = (targetFlags & 0x10u) == 0u;
    } else {
        const int16_t a = GP4_FIELD(int16_t, rec, 0x90);
        const int16_t b = GP4_FIELD(int16_t, rec, 0x94);
        int16_t difference = static_cast<int16_t>(static_cast<uint16_t>(a) - static_cast<uint16_t>(b));
        if (difference < 0) difference = static_cast<int16_t>(0u - static_cast<uint16_t>(difference));
        uint16_t dx = static_cast<uint16_t>(difference);
        bool away = false;
        if (difference >= 0x200) {
            dx = static_cast<uint16_t>(a);
            away = (b < 0) ? (a > b) : (a < b);
        }
        r->edx = (r->edx & 0xffff0000u) | dx;
        if (away) {
            first = b >= 0;
        } else {
            first = b < 0;
        }
    }

    if (first) {
        gp4::call_regs(0x0042ba87, *r);
    } else {
        gp4::Regs callee = *r;
        gp4::call_regs(0x0042bae4, callee);
        *r = callee;
    }
}
GP4_IMPL(0x0042a278, FUN_0042a278, "regs(esi:ptr[0x420]:layout=car_0042ba87+car_0042a278, edx:u32) -> (ebx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00403800+state_004059ef+eq_0042ba87")
