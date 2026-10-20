#include <gp4/gp4.h>
#include <gp4/x87.h>

// Converts the integers DAT_0063804c / 00638050 / 00638054 to doubles at +0xb8 / +0xc0 / +0xc8 of the
// body at 0x00622cf8, rotates them with 0x00401f02 (results at +0xa0 / +0xa8 / +0xb0) and rounds
// the three results into the dwords +0xa4 / +0x150 / +0xa8 of the record in esi (the inverse of
// 0x0041d43f). edx is left holding the body address.
// FUNCTION: GP4 0x0041d492
void __cdecl FUN_0041d492(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622cf8);

    GP4_FIELD(double, body, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x0063804c));
    GP4_FIELD(double, body, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638050));
    GP4_FIELD(double, body, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638054));

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401f02, *r);

    GP4_FIELD(int32_t, record, 0xa4) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa0));
    GP4_FIELD(int32_t, record, 0x150) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa8));
    GP4_FIELD(int32_t, record, 0xa8) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb0));
}
GP4_IMPL(0x0041d492, FUN_0041d492, "regs(esi:ptr[0x158]:bytes) -> (edx); globals=fuzz:i32+layout=view_00609860")
