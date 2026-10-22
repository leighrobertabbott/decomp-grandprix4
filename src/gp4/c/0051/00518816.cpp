#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sound-sample release behind 0x00518538 (eax = the sample index 0..99). Guarded by the re-entry
// lock byte [0x95405c]: when it is already 0xff the routine returns at once with CF set (busy).
// Otherwise the lock is taken, bit 7 of the sample's flag byte 0x88fb28[eax] is cleared and, when the
// channel mapped to the sample (0x88f998[eax]) still shows this sample as its status
// (0x88f948[channel] == eax), the channel is released with 0x518ba7 (eax = channel, ebx = channel).
// The lock is cleared and CF is cleared. pushal/popal and the segment register pushes restore
// everything; popal leaves the flags of the last `cmp` (or of 0x518ba7), then `clc` / `stc`.
// FUNCTION: GP4 0x00518816
void __cdecl FUN_00518816(gp4::Regs* r) {
    const uint32_t sample = r->eax;
    const uint8_t lock = GP4_GLOBAL(uint8_t, 0x0095405c);
    uint32_t f = gp4::flags::sub8(lock, 0xffu);
    if (lock == 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, f | gp4::flags::CF);
        return;
    }

    GP4_GLOBAL(uint8_t, 0x0095405c) = 0xffu;
    GP4_ARRAY(uint8_t, 0x0088fb28)[sample] &= 0x7fu;
    const uint32_t channel = GP4_ARRAY(uint32_t, 0x0088f998)[sample];
    const uint32_t status = GP4_ARRAY(uint32_t, 0x0088f948)[channel];
    f = gp4::flags::sub32(sample, status);
    if (sample == status) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = channel;
        c.ebx = channel;
        c.eflags = gp4::flags::merge(r->eflags, f);
        gp4::call_regs(0x00518ba7, c);
        f = c.eflags;
    }
    GP4_GLOBAL(uint8_t, 0x0095405c) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, f & ~static_cast<uint32_t>(gp4::flags::CF));
}
GP4_IMPL(0x00518816, FUN_00518816, "regs(eax:u32{=0,1,7,50,99,13,64}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=chan_005188f5+chan_00518654+rel_00518816")
