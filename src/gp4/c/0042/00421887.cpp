#include <gp4/gp4.h>
#include <gp4/x87.h>

// Converts the integers DAT_00607f40 / 00607f44 / 00607f48 to doubles at +0xa0 / +0xa8 / +0xb0 of the
// body at 0x00622e98, multiplies them with 0x00401e2e (results at +0xb8 / +0xc0 / +0xc8) and rounds
// the three results into DAT_00607fd0 / 00607fd4 / 00607fd8. edx is left holding the body address.
// FUNCTION: GP4 0x00421887
void __cdecl FUN_00421887(gp4::Regs* r) {
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622e98);

    GP4_FIELD(double, body, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607f40));
    GP4_FIELD(double, body, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607f44));
    GP4_FIELD(double, body, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607f48));

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401e2e, *r);

    GP4_GLOBAL(int32_t, 0x00607fd0) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb8));
    GP4_GLOBAL(int32_t, 0x00607fd4) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc0));
    GP4_GLOBAL(int32_t, 0x00607fd8) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));
}
GP4_IMPL(0x00421887, FUN_00421887, "regs() -> (edx); globals=fuzz:i32+layout=view_00609860")
