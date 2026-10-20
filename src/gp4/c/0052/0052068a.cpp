#include <gp4/gp4.h>

// Resets the 0x007a9124.. block of the current device packet: the second entry of the dword table
// 0x00679b9c is latched into 0x00679b34 and the matching slot of 0x00679b24 is set to -1 (edx = 1
// is the index, eax the latched value); then 0x007a9128..0x007a914c get -1 / 0 constants (-1, -1,
// 0, -1, -1, 0, -1, -1, 0, 0) and 0x007a9124 is cleared.
static __declspec(noinline) uint32_t LatchEntry(uint32_t index) {
    const uint32_t value = GP4_ARRAY(uint32_t, 0x00679b9c)[index];
    GP4_ARRAY(uint32_t, 0x00679b34)[index] = value;
    GP4_ARRAY(uint32_t, 0x00679b24)[index] = 0xffffffffu;
    return value;
}

// FUNCTION: GP4 0x0052068a
void __cdecl FUN_0052068a(gp4::Regs* r) {
    r->edx = 1u;
    r->eax = LatchEntry(1u);
    GP4_GLOBAL(uint32_t, 0x007a9128) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a912c) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9130) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a9134) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9138) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a913c) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a9140) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9144) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x007a9148) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a914c) = 0u;
    GP4_GLOBAL(uint32_t, 0x007a9124) = 0u;
}
GP4_IMPL(0x0052068a, FUN_0052068a, "regs() -> (eax, edx); globals=fuzz:i32+layout=val_0052068a")
