#include <gp4/gp4.h>

// FUNCTION: GP4 0x004104b1
void __cdecl FUN_004104b1(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* destination = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t value = GP4_FIELD(uint8_t, source, 0x32);
    GP4_FIELD(uint8_t, destination, 0x7f) = value;
    r->eax = (r->eax & 0xffffff00u) | value;
}
GP4_IMPL(0x004104b1, FUN_004104b1, "regs(eax:u32, ebx:ptr[0x40]:bytes, esi:ptr[0x87]:bytes) -> (eax)")
