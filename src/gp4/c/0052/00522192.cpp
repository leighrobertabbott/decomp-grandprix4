#include <gp4/gp4.h>
#include <gp4/flags.h>

// Record flag update for the car/driver record at esi. While the byte 0x0066d8e4 is 0xff: without
// bit 0x10 of +0x6f it sets bit 0x20 of +0x8d (and, when bit 4 or bit 8 of +0x6f is set, calls the
// pushal routine 0x00431bf7 with eax = -1 (bit 4) or 1 (bit 8) and edx = [0x0066d8e0]); with bit 0x10
// it clears bit 0x20 of +0x8d. 0x008914e4 is set to 0xff on each of those paths. Always ends with
// +0x8a |= 8 and +0x8b |= 0x40; the flags are those of that last `or`. eax/edx are only changed on the
// call path (the pushal routine preserves them).
// FUNCTION: GP4 0x00522192
void __cdecl FUN_00522192(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if (GP4_GLOBAL(uint8_t, 0x0066d8e4) == 0xffu) {
        if ((rec[0x6f] & 0x10u) == 0u) {
            rec[0x8d] = static_cast<uint8_t>(rec[0x8d] | 0x20u);
            GP4_GLOBAL(uint8_t, 0x008914e4) = 0xffu;
            if ((rec[0x6f] & 4u) != 0u || (rec[0x6f] & 8u) != 0u) {
                gp4::Regs c = *r;
                c.eax = ((rec[0x6f] & 4u) != 0u) ? 0xffffffffu : 1u;
                c.edx = GP4_GLOBAL(uint32_t, 0x0066d8e0);
                gp4::call_regs(0x00431bf7, c);
                r->eax = c.eax;
                r->edx = c.edx;
                GP4_GLOBAL(uint8_t, 0x008914e4) = 0xffu;
            }
        } else {
            rec[0x8d] = static_cast<uint8_t>(rec[0x8d] & 0xdfu);
            GP4_GLOBAL(uint8_t, 0x008914e4) = 0xffu;
        }
    }

    rec[0x8a] = static_cast<uint8_t>(rec[0x8a] | 8u);
    rec[0x8b] = static_cast<uint8_t>(rec[0x8b] | 0x40u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(rec[0x8b]));
}
GP4_IMPL(0x00522192, FUN_00522192, "regs(eax:u32, edx:u32, esi:ptr[0x1e8]:layout=lvl_00431d1f) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=counter_0040f0dd+tbl_0066d94c+levels_006f2cc4+trk_007ad894+circ_007adb50+sel_00522192")
