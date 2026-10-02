#include <gp4/gp4.h>

// FUNCTION: GP4 0x00424865
void __cdecl FUN_00424865(gp4::Regs* r) {
    // clears 0x1e dwords (one 0x78-byte record) at ebx; eax/edi are saved and restored
    uint32_t* record = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->ebx));
    for (uint32_t i = 0; i < 0x1eu; i++) {
        record[i] = 0u;
    }
    r->ecx = 0u;
}
GP4_IMPL(0x00424865, FUN_00424865, "regs(ebx:ptr[0x80]:bytes) -> (ecx)")
