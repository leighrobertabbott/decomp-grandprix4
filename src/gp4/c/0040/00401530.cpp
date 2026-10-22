#include <gp4/gp4.h>

// FUNCTION: GP4 0x00401530
// Copies the three words 0x00629bca/cc/ce to 0x0062d1b2/b4/b6 with push/pop word pairs, so every
// register and flag comes back as the caller passed it (the caller at 0x5201ba goes on to read al).
void __cdecl FUN_00401530(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint16_t, 0x0062d1b2) = GP4_GLOBAL(uint16_t, 0x00629bca);
    GP4_GLOBAL(uint16_t, 0x0062d1b4) = GP4_GLOBAL(uint16_t, 0x00629bcc);
    GP4_GLOBAL(uint16_t, 0x0062d1b6) = GP4_GLOBAL(uint16_t, 0x00629bce);
}
GP4_IMPL(0x00401530, FUN_00401530, "regs(eax:u32) -> (eax); globals=fuzz:i32")
