#include <gp4/gp4.h>

// FUNCTION: GP4 0x00406175
void __cdecl FUN_00406175(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = GP4_FIELD(uint8_t, record, 0xfd);
}
GP4_IMPL(0x00406175, FUN_00406175, "regs(esi:ptr[0x105]:bytes) -> (eax)")
