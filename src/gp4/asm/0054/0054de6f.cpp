#include <gp4/gp4.h>

// Copies the dword at [edi] to [edi+8] and leaves it in eax (`mov eax,[edi]; mov [edi+8],eax; ret`).
// FUNCTION: GP4 0x0054de6f
void __cdecl FUN_0054de6f(gp4::Regs* r) {
    int32_t* rec = reinterpret_cast<int32_t*>(static_cast<uintptr_t>(r->edi));
    const int32_t value = rec[0];
    rec[2] = value;
    r->eax = static_cast<uint32_t>(value);
}
GP4_IMPL(0x0054de6f, FUN_0054de6f, "regs(edi:ptr[0x40]:bytes) -> (eax)")
