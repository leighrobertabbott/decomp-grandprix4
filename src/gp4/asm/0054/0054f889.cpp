#include <gp4/gp4.h>

// Copies the 0xf68-byte block at 0x007ab760 (the first 'Mika Hakkinen' record block) to the buffer
// pointed to by the dword at 0x0083e498 (`rep movsb`, direction flag clear). Leaves esi/edi just
// past both blocks and ecx = 0; the flags are not touched.
// FUNCTION: GP4 0x0054f889
void __cdecl FUN_0054f889(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x007ab760);
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0083e498)));
    for (uint32_t i = 0; i != 0xf68u; ++i) {
        dst[i] = src[i];
    }
    r->esi = 0x007ab760u + 0xf68u;
    r->edi = GP4_GLOBAL(uint32_t, 0x0083e498) + 0xf68u;
    r->ecx = 0;
}
GP4_IMPL(0x0054f889, FUN_0054f889, "regs() -> (esi, edi, ecx); globals=fuzz:i32+layout=fill_0054f889")
