#include <gp4/gp4.h>

// FUNCTION: GP4 0x004086f9
void __cdecl FUN_004086f9(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint8_t, record, 0x86) = 0;
    GP4_FIELD(uint8_t, record, 0x87) = 0;
}
GP4_IMPL(0x004086f9, FUN_004086f9, "regs(esi:ptr[0x8f]:bytes) -> ()")
