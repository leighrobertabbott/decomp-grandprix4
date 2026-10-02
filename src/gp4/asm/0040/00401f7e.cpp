#include <gp4/gp4.h>

// FUNCTION: GP4 0x00401f7e
void __cdecl FUN_00401f7e(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));
    GP4_FIELD(double, body, 0xa8) = GP4_FIELD(double, body, 0xc0) * GP4_FIELD(double, body, 0x78);
}
GP4_IMPL(0x00401f7e, FUN_00401f7e, "regs(edx:ptr[0xc8]:f64) -> ()")
