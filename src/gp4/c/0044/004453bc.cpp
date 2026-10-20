#include <gp4/gp4.h>

// Resets the opcode stream cursors 0x0062d19c / 0x0062d1a0 to the buffer start 0x0062b19a.
// FUNCTION: GP4 0x004453bc
void __cdecl FUN_004453bc() {
    GP4_GLOBAL(uint32_t, 0x0062d19c) = 0x0062b19au;
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = 0x0062b19au;
}
GP4_IMPL(0x004453bc, FUN_004453bc, "cdecl() -> void; globals=fuzz:i32")
