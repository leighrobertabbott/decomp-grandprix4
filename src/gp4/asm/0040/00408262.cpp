#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408262
void __cdecl FUN_00408262(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x90)));
}
GP4_IMPL(0x00408262, FUN_00408262, "regs(esi:ptr[0x98]:bytes) -> (eax)")
