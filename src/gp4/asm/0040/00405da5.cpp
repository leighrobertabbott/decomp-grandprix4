#include <gp4/gp4.h>

// FUNCTION: GP4 0x00405da5
void __cdecl FUN_00405da5(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    r->eax = GP4_FIELD(uint32_t, record, 0x44);
}
GP4_IMPL(0x00405da5, FUN_00405da5, "regs(edi:ptr[0x4c]:bytes) -> (eax)")
