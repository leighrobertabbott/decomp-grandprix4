#include <gp4/gp4.h>

// Copies the three dwords at +0xf0 / +0xf4 / +0xf8 of the edi record into the globals
// 0x870640 / 0x870644 / 0x870648; eax keeps the last one. (0x553d5f is the inverse split.)
// FUNCTION: GP4 0x00553d3d
void __cdecl FUN_00553d3d(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    GP4_GLOBAL(uint32_t, 0x00870640) = GP4_FIELD(uint32_t, record, 0xf0);
    GP4_GLOBAL(uint32_t, 0x00870644) = GP4_FIELD(uint32_t, record, 0xf4);
    const uint32_t last = GP4_FIELD(uint32_t, record, 0xf8);
    GP4_GLOBAL(uint32_t, 0x00870648) = last;
    r->eax = last;
}
GP4_IMPL(0x00553d3d, FUN_00553d3d, "regs(edi:ptr[0x100]:bytes) -> (eax)")
