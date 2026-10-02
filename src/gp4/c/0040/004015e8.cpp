#include <gp4/gp4.h>

// FUNCTION: GP4 0x004015e8
void __cdecl FUN_004015e8(gp4::Regs*) {
    GP4_GLOBAL(uint8_t, 0x0062d1b3) = GP4_GLOBAL(uint8_t, 0x0066d8bf);
    GP4_GLOBAL(uint8_t, 0x0062d1b4) = GP4_GLOBAL(uint8_t, 0x0066d8c0);
    GP4_GLOBAL(uint8_t, 0x0062d1b5) = GP4_GLOBAL(uint8_t, 0x0066d8c1);
    GP4_GLOBAL(uint8_t, 0x0062d1b6) = GP4_GLOBAL(uint8_t, 0x0066d8c2);
    GP4_GLOBAL(uint8_t, 0x0062d1b7) = GP4_GLOBAL(uint8_t, 0x0066d8c3);
}
GP4_IMPL(0x004015e8, FUN_004015e8, "regs() -> (); globals=layout=state_0066d8c0")
