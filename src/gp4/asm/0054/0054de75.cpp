#include <gp4/gp4.h>

// Copies the dword at [edi+4] to [edi+8] and leaves it in eax (`mov eax,[edi+4]; mov [edi+8],eax; ret`).
// FUNCTION: GP4 0x0054de75
void __cdecl FUN_0054de75(gp4::Regs* r) {
    int32_t* rec = reinterpret_cast<int32_t*>(static_cast<uintptr_t>(r->edi));
    const int32_t value = rec[1];
    rec[2] = value;
    r->eax = static_cast<uint32_t>(value);
}
GP4_IMPL(0x0054de75, FUN_0054de75, "regs(edi:ptr[0x40]:bytes) -> (eax)")
