#include <gp4/gp4.h>
#include <gp4/x87.h>

// Loads three integer globals as doubles into the vector at +0xa0 / +0xa8 / +0xb0 of the body
// pointed to by [[0x00609860]+0x40], lets 0x00401e2e multiply it by the body's matrix and rounds the
// results at +0xb8 / +0xc0 / +0xc8 into the dwords +0x30 / +0x34 / +0x38 of the record. edi is
// left at the record and edx at the body.
// FUNCTION: GP4 0x00425395
void __cdecl FUN_00425395(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609860)));
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, record, 0x40)));
    GP4_FIELD(double, body, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607f40));
    GP4_FIELD(double, body, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607f44));
    GP4_FIELD(double, body, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607f48));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401e2e, *r);
    GP4_FIELD(int32_t, record, 0x30) = fistp32(GP4_FIELD(double, body, 0xb8));
    GP4_FIELD(int32_t, record, 0x34) = fistp32(GP4_FIELD(double, body, 0xc0));
    GP4_FIELD(int32_t, record, 0x38) = fistp32(GP4_FIELD(double, body, 0xc8));
}
GP4_IMPL(0x00425395, FUN_00425395, "regs() -> (edi, edx); globals=layout=view_00609860")
