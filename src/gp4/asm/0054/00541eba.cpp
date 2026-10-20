#include <gp4/gp4.h>

// Stores esi in 0x007a9788, clears the byte 0x007a9790 and copies the dword at 0x007a9784 to
// 0x007a9780; eax keeps the copied value. The flags are not touched.
// FUNCTION: GP4 0x00541eba
void __cdecl FUN_00541eba(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x007a9788) = r->esi;
    GP4_GLOBAL(uint8_t, 0x007a9790) = 0u;
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x007a9784);
    GP4_GLOBAL(uint32_t, 0x007a9780) = value;
    r->eax = value;
}
GP4_IMPL(0x00541eba, FUN_00541eba, "regs(esi:u32) -> (eax); globals=fuzz:i32")
