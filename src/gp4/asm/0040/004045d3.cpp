#include <gp4/gp4.h>

// FUNCTION: GP4 0x004045d3
void __cdecl FUN_004045d3(gp4::Regs* r) {
    const uint32_t* record = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = record[0];
    r->edx = record[1];
}
GP4_IMPL(0x004045d3, FUN_004045d3, "regs(esi:ptr[0xc]:bytes) -> (eax, edx)")
