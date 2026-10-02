#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408351
void __cdecl FUN_00408351(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = GP4_FIELD(uint8_t, record, 0x202);
}
GP4_IMPL(0x00408351, FUN_00408351, "regs(esi:ptr[0x20a]:bytes) -> (eax)")
