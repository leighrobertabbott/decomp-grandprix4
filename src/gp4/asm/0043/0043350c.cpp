#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043350c
void __cdecl FUN_0043350c(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);
    GP4_FIELD(uint16_t, body, 0x9a) = 0;
    GP4_FIELD(uint32_t, body, 0xa4) = 0;
    GP4_FIELD(uint32_t, body, 0xa8) = 0;
    GP4_FIELD(uint32_t, body, 0xac) = 0;
    GP4_FIELD(uint32_t, body, 0x158) = 0;
}
GP4_IMPL(0x0043350c, FUN_0043350c, "regs(esi:ptr[0x160]:bytes) -> ()")
