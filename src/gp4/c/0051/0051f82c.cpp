#include <gp4/gp4.h>

// `pushal ; call 0x0052a260 ; mov [edx + eax], cl ; popal ; ret`: 0x0052a260 turns (eax = record,
// ebx = entry) into the address of the 0xe4-byte entry inside the 0xf28-byte record array at
// 0x007adb4c; cl is stored at that address + edx. popal restores every general register and the
// flags stay those of the callee's final `add`.
// FUNCTION: GP4 0x0051f82c
void __cdecl FUN_0051f82c(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0052a260, c);
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edx + c.eax)) = static_cast<uint8_t>(c.ecx);
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051f82c, FUN_0051f82c, "regs(eax:u32[0..25], ebx:u32[0..15], ecx:u32, edx:u32[0..0xe3]) -> (cf, pf, af, zf, sf, of)")
