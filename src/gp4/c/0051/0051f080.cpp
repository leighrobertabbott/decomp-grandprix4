#include <gp4/gp4.h>

static __declspec(noinline) uint32_t slotIndex() {
    return 0x3du;
}

// Copies entry 0x3d of the dword table at 0x00891bd0 to the dword 0x00852d38. eax is saved and
// restored, so no register or flag changes.
// FUNCTION: GP4 0x0051f080
void __cdecl FUN_0051f080(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint32_t, 0x00852d38) = GP4_ARRAY(uint32_t, 0x00891bd0)[slotIndex()];
}
GP4_IMPL(0x0051f080, FUN_0051f080, "regs() -> (); globals=fuzz:i32")
