#include <gp4/gp4.h>

// FUNCTION: GP4 0x00419e97
void __cdecl FUN_00419e97(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00623e18) = GP4_GLOBAL(uint32_t, 0x006228e8) + GP4_GLOBAL(uint32_t, 0x00607b0c);
    GP4_GLOBAL(uint32_t, 0x00623e1c) = GP4_GLOBAL(uint32_t, 0x006228ec) + GP4_GLOBAL(uint32_t, 0x00607b10);
    GP4_GLOBAL(uint32_t, 0x00623e20) = GP4_GLOBAL(uint32_t, 0x006228f0) + GP4_GLOBAL(uint32_t, 0x00607b14);

    // the callee leaves ebp, edx and the flags of its final add; they stay in the registers
    gp4::call_regs(0x00401adc, *r);

    GP4_GLOBAL(uint32_t, 0x006228dc) = GP4_GLOBAL(uint32_t, 0x00623e24);
    GP4_GLOBAL(uint32_t, 0x006228e0) = GP4_GLOBAL(uint32_t, 0x00623e28);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00623e2c);
    GP4_GLOBAL(uint32_t, 0x006228e4) = last;
    r->eax = last;
}
GP4_IMPL(0x00419e97, FUN_00419e97, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
