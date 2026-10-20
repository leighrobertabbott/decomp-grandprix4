#include <gp4/gp4.h>
#include <gp4/flags.h>

// eax = (eax + 7) * g + (g >> 1) with g the dword at 0x00b1bf80 (a rounded scale: the offset slot
// count times g, plus half g). ebx is saved/restored; the flags are those of the final `add eax, ebx`.
// FUNCTION: GP4 0x00549cfb
void __cdecl FUN_00549cfb(gp4::Regs* r) {
    const uint32_t scale = GP4_GLOBAL(uint32_t, 0x00b1bf80);

    const uint32_t scaled = (r->eax + 7u) * scale;
    const uint32_t half = scale >> 1;
    const uint32_t result = scaled + half;

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(scaled, half));
}
GP4_IMPL(0x00549cfb, FUN_00549cfb, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
