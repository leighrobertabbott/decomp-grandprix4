#include <gp4/gp4.h>

// Fills the 5-byte block at 0x0089152c with 0x20, 2, the byte at 0x0088fda0, 7, 1 (stosb sequence).
// Leaves al = 1 (upper eax kept) and edi = 0x00891531; the flags are untouched.
// FUNCTION: GP4 0x00516927
void __cdecl FUN_00516927(gp4::Regs* r) {
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x0089152c);

    *dst++ = 0x20;
    *dst++ = 2;
    *dst++ = GP4_GLOBAL(uint8_t, 0x0088fda0);
    *dst++ = 7;
    *dst++ = 1;

    r->eax = (r->eax & 0xffffff00u) | 1u;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
}
GP4_IMPL(0x00516927, FUN_00516927, "regs(eax:u32) -> (eax, edi); globals=fuzz:i32")
