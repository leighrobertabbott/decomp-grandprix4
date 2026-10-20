#include <gp4/gp4.h>

// Loads esi with the address of the byte buffer at 0x00843fc5 and returns eax = 0; no flags are
// touched.
// FUNCTION: GP4 0x005327ec
void __cdecl FUN_005327ec(gp4::Regs* r) {
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00843fc5)));
    r->eax = 0u;
}
GP4_IMPL(0x005327ec, FUN_005327ec, "regs() -> (eax, esi)")
