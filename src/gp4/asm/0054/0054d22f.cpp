#include <gp4/gp4.h>

// `lodsb` then the colour setter 0x0054cc37: al = *esi++ (upper eax bytes kept) and the registers
// go straight to 0x0054cc37, whose outputs (eax, ebx, flags) are this routine's.
// FUNCTION: GP4 0x0054d22f
void __cdecl FUN_0054d22f(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = (r->eax & 0xffffff00u) | *p;
    r->esi = r->esi + 1u;
    gp4::call_regs(0x0054cc37, *r);
}
GP4_IMPL(0x0054d22f, FUN_0054d22f, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x40]:u8[0..0x30]) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=col_0054cc37")
