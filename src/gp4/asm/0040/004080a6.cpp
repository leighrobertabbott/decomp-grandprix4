#include <gp4/gp4.h>

// FUNCTION: GP4 0x004080a6
void __cdecl FUN_004080a6(gp4::Regs* r) {
    // pushal ... popal restores every register, but not the flags left by the last 0x401462 call
    gp4::Regs saved = *r;
    gp4::Regs call = *r;
    uint32_t count = r->eax & 0xfffu;
    if (count < 0xau) {
        count = 0xau;
    }
    call.eax = count;
    // mov ecx, eax ; loop: call 0x401462 (advances the generator state) ecx times
    for (uint32_t i = 0; i != count; ++i) {
        gp4::call_regs(0x00401462, call);
    }
    saved.eflags = call.eflags;
    *r = saved;
}
GP4_IMPL(0x004080a6, FUN_004080a6, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
