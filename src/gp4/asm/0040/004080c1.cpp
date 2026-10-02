#include <gp4/gp4.h>

// FUNCTION: GP4 0x004080c1
void __cdecl FUN_004080c1(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = GP4_FIELD(uint8_t, record, 0x226);
}
GP4_IMPL(0x004080c1, FUN_004080c1, "regs(esi:ptr[0x22e]:bytes) -> (eax)")
