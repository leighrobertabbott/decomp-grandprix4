#include <gp4/gp4.h>
#include <gp4/flags.h>

// Waiting loop (pushal/popal): stores ax in the word counter [0x007a9470], then repeats the empty hook
// 0x005ae990, the Windows message pump 0x00509b50, the periodic update 0x005281fe and the wait/notify
// routine 0x00549e2c while the signed counter is above zero (the counter is counted down outside this
// loop, e.g. by the message handlers). popal restores every register; the flags are those of the last
// `cmp word [0x007a9470], 0`.
// FUNCTION: GP4 0x0052cb10
void __cdecl FUN_0052cb10(gp4::Regs* r) {
    gp4::Regs c = *r;
    GP4_GLOBAL(uint16_t, 0x007a9470) = static_cast<uint16_t>(r->eax);
    uint32_t counter;
    do {
        gp4::call_regs(0x005ae990, c);
        gp4::call_regs(0x00509b50, c);
        gp4::call_regs(0x005281fe, c);
        gp4::call_regs(0x00549e2c, c);
        counter = GP4_GLOBAL(uint16_t, 0x007a9470);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(counter, 0u));
    } while (static_cast<int16_t>(counter) > 0);
}
GP4_IMPL(0x0052cb10, FUN_0052cb10, "regs(eax:u32{=0,0xffff,0xfff0,0x8000}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
