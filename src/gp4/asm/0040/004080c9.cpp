#include <gp4/gp4.h>

// FUNCTION: GP4 0x004080c9
void __cdecl FUN_004080c9(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = GP4_FIELD(uint16_t, record, 0x224);
}
GP4_IMPL(0x004080c9, FUN_004080c9, "regs(esi:ptr[0x22c]:bytes) -> (eax)")
