#include <gp4/gp4.h>

// mov edi, [0x8245c0] ; mov [0x8245c4], edi ; ret: copies the key-queue write cursor over the
// read cursor (empties the queue) and leaves it in edi.
// FUNCTION: GP4 0x0052d813
void __cdecl FUN_0052d813(gp4::Regs* r) {
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x008245c0);
    GP4_GLOBAL(uint32_t, 0x008245c4) = cursor;
    r->edi = cursor;
}
GP4_IMPL(0x0052d813, FUN_0052d813, "regs() -> (edi); globals=fuzz:i32")
