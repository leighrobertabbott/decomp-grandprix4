#include <gp4/gp4.h>

// `lodsb ; movzx eax, al ; call 0x0054cb71 ; ret`: eax = *esi++ (zero extended). The callee
// (push/pop of eax, esi, edi) changes nothing. Flags are untouched.
// FUNCTION: GP4 0x0054d27b
void __cdecl FUN_0054d27b(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t al = *p;
    r->esi = r->esi + 1u;
    r->eax = al;
    gp4::call_regs(0x0054cb71, *r);
}
GP4_IMPL(0x0054d27b, FUN_0054d27b, "regs(esi:ptr[0x40]:bytes) -> (eax, esi)")
