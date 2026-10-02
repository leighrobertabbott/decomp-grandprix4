#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040825e
void __cdecl FUN_0040825e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = GP4_FIELD(uint32_t, record, 0x1c);
}
GP4_IMPL(0x0040825e, FUN_0040825e, "regs(esi:ptr[0x24]:bytes) -> (eax)")
