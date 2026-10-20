#include <gp4/gp4.h>
#include <gp4/flags.h>

// 0x007c2e72 - 0x007ad890: the offset of the byte inside the saved-state buffer [0x007a93e0] (a copy of
// the block that starts at 0x007ad890).
static __declspec(noinline) uint32_t blockOffset(uint32_t address, uint32_t blockStart) {
    return address - blockStart;
}

// pushal wrapper: when the saved-state buffer byte for 0x007c2e72 is 0xff, runs command 0x42 of the
// dispatcher 0x00531f9c (state-file save) and returns CF = 0 (clc); otherwise CF = 1 (stc) and the
// other flags are those of `cmp byte [esi], 0xff`. popal restores every GPR; with the command run the
// flags are those the dispatcher leaves, with CF cleared.
// FUNCTION: GP4 0x005121f4
void __cdecl FUN_005121f4(gp4::Regs* r) {
    const uint8_t* slot = (const uint8_t*)(uintptr_t)(
        blockOffset(0x007c2e72u, 0x007ad890u) + GP4_GLOBAL(uint32_t, 0x007a93e0));
    const uint32_t value = *slot;
    if (value != 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, 0xffu) | gp4::flags::CF);
        return;
    }
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, 0xffu));
    c.eax = (c.eax & 0xffff0000u) | 0x42u;
    gp4::call_regs(0x00531f9c, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~static_cast<uint32_t>(gp4::flags::CF));
}
GP4_IMPL(0x005121f4, FUN_005121f4, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=save_005359c8")
