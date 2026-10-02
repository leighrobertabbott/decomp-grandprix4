#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043cbac
void __cdecl FUN_0043cbac(gp4::Regs* r) {
    gp4::Regs call = *r;
    uint32_t lastFlags = r->eflags;

    for (uint32_t row = 0; row < 0x16; ++row) {
        const uint32_t base = row * 0x2c;
        for (uint32_t column = 0; column < 0x2c; ++column) {
            GP4_ARRAY(uint8_t, 0x0066d962)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066dd2a)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066e0f2)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066e4ba)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066e882)[base + column] = 0;
            GP4_ARRAY(uint32_t, 0x0066ec60)[base + column] = 0;
        }
        GP4_ARRAY(uint8_t, 0x0066d936)[row] = 0;
        GP4_ARRAY(uint8_t, 0x0066d94c)[row] = 0xff;
        // eax still holds (row * 0x2c) << 2 from the clearing loop when the generator is called.
        call.eax = base << 2;
        gp4::call_regs(0x00401462, call);
        GP4_ARRAY(uint8_t, 0x0066fd20)[row] = static_cast<uint8_t>(call.eax);
        lastFlags = call.eflags;
    }

    // popal restores every register. The last flag writer is `inc ebx` (21 -> 22), which
    // clears PF/AF/ZF/SF/OF and keeps the carry left by the last generator call.
    r->eflags = (r->eflags & ~0x8d5u) | (lastFlags & 1u);
}
GP4_IMPL(0x0043cbac, FUN_0043cbac, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
