#include <gp4/gp4.h>

// Resets the session-setup state before the driver slots are bound to the car records. While the byte
// [0x007a8dfe] is 0xff (first call) the 22-byte slot key table at 0x007c2a1c is copied to 0x0062f916 and
// [0x007a8dfb] / [0x0062f8d8] are cleared. Then the counters and flags of the block 0x0062f8ac..0x0062f974
// and 0x007a8df9..0x007a8e00 are zeroed, 0x4456c7 ranks the slots, the selected-car pointer [0x0062f8b4] is set
// to the car array 0x0066fef0, 0x44576b marks the participating cars and [0x007a8dfe] becomes 0.
// pushal / popal restore every register; the flags are those of the last add in 0x44576b.
// FUNCTION: GP4 0x0044557c
void __cdecl FUN_0044557c(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x007a8dfe) == 0xffu) {
        const uint8_t* source = GP4_ARRAY(const uint8_t, 0x007c2a1c);
        uint8_t* target = GP4_ARRAY(uint8_t, 0x0062f916);
        for (uint32_t i = 0; i != 0x16u; ++i) {
            target[i] = source[i];
        }
        GP4_GLOBAL(uint8_t, 0x007a8dfb) = 0;
        GP4_GLOBAL(uint32_t, 0x0062f8d8) = 0;
    }

    GP4_GLOBAL(uint32_t, 0x0062f8ac) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f8bc) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f8c0) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f8c4) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f8cc) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8e00) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f8dc) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f8e0) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8df9) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8dfa) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8dfc) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8dfd) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8dff) = 0;
    GP4_GLOBAL(uint8_t, 0x0062f8ff) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f93c) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f940) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f948) = 0;
    GP4_GLOBAL(uint8_t, 0x0062f8f8) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f944) = 0;
    GP4_GLOBAL(uint8_t, 0x0062f8f9) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f94c) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f950) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f968) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f96c) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f970) = 0;
    GP4_GLOBAL(uint32_t, 0x0062f974) = 0;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004456c7, call);

    GP4_GLOBAL(uint32_t, 0x0062f8b4) = 0x0066fef0u;

    gp4::call_regs(0x0044576b, call);

    GP4_GLOBAL(uint8_t, 0x007a8dfe) = 0;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0044557c, FUN_0044557c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c")
