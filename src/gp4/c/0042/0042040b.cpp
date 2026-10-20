#include <gp4/gp4.h>
#include <gp4/x87.h>

// Converts the integers DAT_006228d0 / 006228d4 / 006228d8 to doubles at +0xa0 / +0xa8 / +0xb0 of the
// body at 0x00622e98, multiplies them with 0x00401e2e (results at +0xb8 / +0xc0 / +0xc8) and rounds
// the three results into DAT_0062290c / 00622910 / 00622914. edx is left holding the body address.
// FUNCTION: GP4 0x0042040b
void __cdecl FUN_0042040b(gp4::Regs* r) {
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622e98);

    GP4_FIELD(double, body, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006228d0));
    GP4_FIELD(double, body, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006228d4));
    GP4_FIELD(double, body, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006228d8));

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401e2e, *r);

    GP4_GLOBAL(int32_t, 0x0062290c) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb8));
    GP4_GLOBAL(int32_t, 0x00622910) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc0));
    GP4_GLOBAL(int32_t, 0x00622914) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));
}
GP4_IMPL(0x0042040b, FUN_0042040b, "regs() -> (edx); globals=fuzz:i32+layout=view_00609860")
