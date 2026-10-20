#include <gp4/gp4.h>

// Copies the dword at +0x28 of the esi record into the global at 0x00866070; eax keeps the value.
// FUNCTION: GP4 0x00515454
void __cdecl FUN_00515454(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(r->esi);
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x28);
    GP4_GLOBAL(uint32_t, 0x00866070) = value;
    r->eax = value;
}
GP4_IMPL(0x00515454, FUN_00515454, "regs(esi:ptr[0x30]:bytes) -> (eax)")
