#include <gp4/gp4.h>

// Copies the three 16-bit words at 0x00933a14, 0x00933a16, 0x00933a18 (the saved RGB triple, 16-bit
// channels) to edi, edi+2 and edi+4. eax is pushed and restored; the flags are untouched.
// FUNCTION: GP4 0x0051b094
void __cdecl FUN_0051b094(gp4::Regs* r) {
    uint16_t* out = reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(r->edi));
    out[0] = GP4_GLOBAL(uint16_t, 0x00933a14);
    out[1] = GP4_GLOBAL(uint16_t, 0x00933a16);
    out[2] = GP4_GLOBAL(uint16_t, 0x00933a18);
}
GP4_IMPL(0x0051b094, FUN_0051b094, "regs(edi:ptr[0x10]:bytes) -> (); globals=fuzz:i32")
