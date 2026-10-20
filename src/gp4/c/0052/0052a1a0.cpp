#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the 0x38-byte block at 0x0081d1f0 with entry [0x0081d1d4] of group [0x007ad894] of record
// [0x007c2f28] of the array at 0x007adb4c (address from 0x0052a279). When the byte [0x0081d1e8] is not
// 0xff and the two differ, [0x0081d1d0] = 0xff, 0x0052a22b rebuilds the block and it is copied (0x38
// bytes) over the entry. The array walker 0x0052ab04 runs before and after; the middle part is a
// pushal/popal block, so the registers are unchanged and the flags are those of the closing
// 0x0052ab04.
// FUNCTION: GP4 0x0052a1a0
void __cdecl FUN_0052a1a0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0052ab04, c);

    gp4::Regs d = c;
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0081d1e8);
    d.eflags = gp4::flags::merge(d.eflags, gp4::flags::sub8(mode, 0xffu));
    if (mode != 0xffu) {
        d.eax = GP4_GLOBAL(uint32_t, 0x007c2f28);
        d.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
        d.ecx = GP4_GLOBAL(uint8_t, 0x0081d1d4);
        gp4::call_regs(0x0052a279, d);

        uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(d.eax));
        const uint8_t* ramp = GP4_ARRAY(const uint8_t, 0x0081d1f0);
        bool same = true;
        for (uint32_t i = 0u; i != 0x38u; ++i) {
            d.eflags = gp4::flags::merge(d.eflags, gp4::flags::sub8(ramp[i], entry[i]));
            if (ramp[i] != entry[i]) {
                same = false;
                break;
            }
        }
        if (!same) {
            GP4_GLOBAL(uint8_t, 0x0081d1d0) = 0xffu;
            d.ecx = 0x38u;
            gp4::call_regs(0x0052a22b, d);
            for (uint32_t i = 0u; i != 0x38u; ++i) {
                entry[i] = ramp[i];
            }
        }
    }

    c.eflags = d.eflags;
    gp4::call_regs(0x0052ab04, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052a1a0, FUN_0052a1a0, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+ent_0052a0ec")
