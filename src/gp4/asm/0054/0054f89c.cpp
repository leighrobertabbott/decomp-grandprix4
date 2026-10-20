#include <gp4/gp4.h>

// Restores the 0xf68-byte block at 0x007ab760 from the buffer pointed to by the dword at 0x0083e498
// (`rep movsb`, direction flag clear; the inverse of 0x0054f889). Leaves esi/edi just past both
// blocks and ecx = 0; the flags are not touched.
// FUNCTION: GP4 0x0054f89c
void __cdecl FUN_0054f89c(gp4::Regs* r) {
    const uint32_t srcAddr = GP4_GLOBAL(uint32_t, 0x0083e498);
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(srcAddr));
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007ab760);
    for (uint32_t i = 0; i != 0xf68u; ++i) {
        dst[i] = src[i];
    }
    r->esi = srcAddr + 0xf68u;
    r->edi = 0x007ab760u + 0xf68u;
    r->ecx = 0;
}
GP4_IMPL(0x0054f89c, FUN_0054f89c, "regs() -> (esi, edi, ecx); globals=fuzz:i32+layout=fill_0054f889")
