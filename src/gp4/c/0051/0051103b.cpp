#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the word at 0x007c2e70 is zero, runs 0x0043cd92 (which keeps every register) and stores 0xff at
// 0x007c2e80. The flags are those of the word test, or of the last flag writer inside 0x0043cd92.
// FUNCTION: GP4 0x0051103b
void __cdecl FUN_0051103b(gp4::Regs* r) {
    const uint16_t value = GP4_GLOBAL(uint16_t, 0x007c2e70);
    uint32_t flags = gp4::flags::logic16(value);
    if (value == 0u) {
        gp4::Regs call = *r;
        gp4::call_regs(0x0043cd92, call);
        flags = call.eflags & 0x8d5u;
        GP4_GLOBAL(uint8_t, 0x007c2e80) = 0xffu;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051103b, FUN_0051103b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0")
