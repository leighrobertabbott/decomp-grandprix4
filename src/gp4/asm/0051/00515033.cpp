#include <gp4/gp4.h>

// Copies the dword at +0x38 of the esi record into the global at 0x00866074; eax keeps the value.
// FUNCTION: GP4 0x00515033
void __cdecl FUN_00515033(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(r->esi);
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x38);
    GP4_GLOBAL(uint32_t, 0x00866074) = value;
    r->eax = value;
}
GP4_IMPL(0x00515033, FUN_00515033, "regs(esi:ptr[0x40]:bytes) -> (eax)")
