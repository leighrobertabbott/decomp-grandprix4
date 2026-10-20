#include <gp4/gp4.h>
#include <gp4/flags.h>

// Pops one word from the key queue (0x0052d7e2) and answers whether it is the code 0x21b (eax = 1 or 0).
// The flags are those of `cmp ax, 0x21b`; the `mov eax, n` / `jmp` that follow leave them alone.
// FUNCTION: GP4 0x0052d848
void __cdecl FUN_0052d848(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x0052d7e2, call);
    const uint32_t key = call.eax & 0xffffu;
    r->eax = (key == 0x21bu) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(key, 0x21bu));
}
GP4_IMPL(0x0052d848, FUN_0052d848, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0")
