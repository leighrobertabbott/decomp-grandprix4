#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042192d
void __cdecl FUN_0042192d(gp4::Regs* r) {
    // Copy 14 dwords from the block that [0x6097b0] points to into the area right after that
    // pointer (0x6097b4), then publish the dword at 0x6097cc (copy offset +0x18) in 0x6082b4.
    // "rep movsd" leaves esi/edi past the copied range and ecx at zero (direction flag clear).
    const uint32_t* const source = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006097b0)));
    uint32_t* const dest = GP4_ARRAY(uint32_t, 0x006097b4);
    for (int i = 0; i < 0xe; i++) {
        dest[i] = source[i];
    }
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x006097cc);
    GP4_GLOBAL(uint32_t, 0x006082b4) = value;

    r->eax = value;
    r->ecx = 0;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source + 0xe));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dest + 0xe));
}
GP4_IMPL(0x0042192d, FUN_0042192d, "regs() -> (eax, ecx, esi, edi); globals=layout=state_00422397")
