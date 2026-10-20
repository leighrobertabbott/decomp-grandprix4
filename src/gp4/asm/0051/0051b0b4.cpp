#include <gp4/gp4.h>

// Copies the three 16-bit words at 0x00933a1a, 0x00933a1c, 0x00933a1e (the second saved RGB triple,
// 16-bit channels) to edi, edi+2 and edi+4. eax is pushed and restored; the flags are untouched.
// FUNCTION: GP4 0x0051b0b4
void __cdecl FUN_0051b0b4(gp4::Regs* r) {
    uint16_t* out = reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(r->edi));
    out[0] = GP4_GLOBAL(uint16_t, 0x00933a1a);
    out[1] = GP4_GLOBAL(uint16_t, 0x00933a1c);
    out[2] = GP4_GLOBAL(uint16_t, 0x00933a1e);
}
GP4_IMPL(0x0051b0b4, FUN_0051b0b4, "regs(edi:ptr[0x10]:bytes) -> (); globals=fuzz:i32")
