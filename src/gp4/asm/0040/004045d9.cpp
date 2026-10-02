#include <gp4/gp4.h>

// FUNCTION: GP4 0x004045d9
void __cdecl FUN_004045d9(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(uint32_t, 0x00934df0) = GP4_FIELD(const uint32_t, record, 0x00);
    GP4_GLOBAL(uint32_t, 0x00934df4) = GP4_FIELD(const uint32_t, record, 0x04);
    GP4_GLOBAL(uint16_t, 0x00934de8) = GP4_FIELD(const uint16_t, record, 0x12);
    GP4_GLOBAL(uint32_t, 0x00934df8) = GP4_FIELD(const uint32_t, record, 0x08);
    GP4_GLOBAL(uint16_t, 0x00934e88) = GP4_FIELD(const uint16_t, record, 0x16);
}
GP4_IMPL(0x004045d9, FUN_004045d9, "regs(esi:ptr[0x20]:bytes) -> ()")
