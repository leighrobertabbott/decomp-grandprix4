#include <gp4/gp4.h>

// Stores the three 16-bit words at esi, esi+2 and esi+4 into 0x00933a1a, 0x00933a1c, 0x00933a1e
// (the second saved RGB triple). eax is pushed and restored; the flags are untouched.
// FUNCTION: GP4 0x0051b103
void __cdecl FUN_0051b103(gp4::Regs* r) {
    const uint16_t* in = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(uint16_t, 0x00933a1a) = in[0];
    GP4_GLOBAL(uint16_t, 0x00933a1c) = in[1];
    GP4_GLOBAL(uint16_t, 0x00933a1e) = in[2];
}
GP4_IMPL(0x0051b103, FUN_0051b103, "regs(esi:ptr[0x10]:bytes) -> (); globals=fuzz:i32")
