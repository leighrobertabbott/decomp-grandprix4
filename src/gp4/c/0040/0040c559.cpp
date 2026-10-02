#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040c559
void __cdecl FUN_0040c559(gp4::Regs* r) {
    r->eax = GP4_GLOBAL(uint32_t, 0x0062250c);
    r->edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00622514)));
}
GP4_IMPL(0x0040c559, FUN_0040c559, "regs() -> (eax, edx); globals=fuzz:i32")
