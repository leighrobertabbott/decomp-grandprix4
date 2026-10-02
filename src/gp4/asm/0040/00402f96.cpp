#include <gp4/gp4.h>

// FUNCTION: GP4 0x00402f96
void __cdecl FUN_00402f96(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = record[0x7f];
}
GP4_IMPL(0x00402f96, FUN_00402f96, "regs(esi:ptr[0x87]:bytes) -> (eax)")
