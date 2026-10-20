#include <gp4/gp4.h>

// FUNCTION: GP4 0x0053205f
// Tail wrapper: `call 0x00504bd0 ; ret`. The callee (compiled C, no register input) posts the message
// 0x402 to the main window [0x00b1c7f0], sets the byte 0x00b1d3e1 and runs 0x004ffc50, 0x00509c80 and
// 0x00509ed0; it returns 0 in eax.
uint32_t __cdecl FUN_0053205f() {
    return gp4::cdecl_<0x00504bd0, uint32_t>();
}
GP4_IMPL(0x0053205f, FUN_0053205f, "cdecl() -> u32; globals=fuzz:i32")
