#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040826a
void __cdecl FUN_0040826a(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = GP4_FIELD(uint32_t, record, 0x244);
}
GP4_IMPL(0x0040826a, FUN_0040826a, "regs(esi:ptr[0x24c]:bytes) -> (eax)")
