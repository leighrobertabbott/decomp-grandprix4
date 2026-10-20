#include <gp4/gp4.h>

// `push ebx,ecx,edx,ebp,esi,edi ; call 0x0052a260 ; movzx eax, byte [edx + eax] ; pop ... ; ret`:
// 0x0052a260 turns (eax = record, ebx = entry) into the address of the 0xe4-byte entry inside the
// 0xf28-byte record array at 0x007adb4c; the byte at that address + edx is returned in eax. Every
// register but eax is restored; the flags stay those of the callee's final `add`.
// FUNCTION: GP4 0x0051f816
void __cdecl FUN_0051f816(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0052a260, c);
    r->eax = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.edx + c.eax));
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051f816, FUN_0051f816, "regs(eax:u32[0..21], ebx:u32[0..16], edx:u32[0..0xe3]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=setup_007adb54+circ_007adb50")
