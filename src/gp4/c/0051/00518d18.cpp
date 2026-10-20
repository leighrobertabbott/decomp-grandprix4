#include <gp4/gp4.h>

// Sound-channel scan: unless the byte [0x0095404c] is set, walks the 16 channel status bytes at 0x0088f988.
// A byte equal to 0x80 (claimed) is checked with 0x00518d01 (eax = channel number, returns 0 / -1);
// when that returns 0 the channel's bit is cleared in both masks 0x0088f700 / 0x0088f704 and the status
// byte is reset to 0.
// FUNCTION: GP4 0x00518d18
void __cdecl FUN_00518d18() {
    if (GP4_GLOBAL(uint8_t, 0x0095404c) != 0u) {
        return;
    }
    uint32_t channel = 0u;
    do {
        if (GP4_ARRAY(uint8_t, 0x0088f988)[channel] == 0x80u) {
            gp4::Regs c = {};
            c.eax = channel;
            c.ecx = channel;
            gp4::call_regs(0x00518d01, c);
            if (c.eax == 0u) {
                GP4_GLOBAL(uint32_t, 0x0088f704) &= ~(1u << channel);
                GP4_GLOBAL(uint32_t, 0x0088f700) &= ~(1u << channel);
                GP4_ARRAY(uint8_t, 0x0088f988)[channel] = 0u;
            }
        }
        channel++;
    } while (channel != 0x10u);
}
GP4_IMPL(0x00518d18, FUN_00518d18, "cdecl() -> void; globals=fuzz:i32+layout=full_00518b09")
