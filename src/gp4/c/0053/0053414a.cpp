#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills the eight-dword table at 0x00828214 with the value 0x0054d1ad returns for index 0 (the first
// entry of its dword table). esi is saved and restored; ecx ends 0 and edi at the end of the table.
// The flags are those of the final `dec ecx` (1 -> 0), carrying the CF of the last `add edi, 4`.
// FUNCTION: GP4 0x0053414a
void __cdecl FUN_0053414a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = 0u;
    gp4::call_regs(0x0054d1ad, c);

    uint32_t* table = GP4_ARRAY(uint32_t, 0x00828214);
    for (uint32_t i = 0u; i < 8u; ++i) {
        table[i] = c.esi;
    }
    r->ecx = 0u;
    r->edi = 0x00828214u + 0x20u;
    r->eflags = gp4::flags::merge(r->eflags,
        gp4::flags::dec32(1u, gp4::flags::add32(0x00828214u + 0x1cu, 4u)));
}
GP4_IMPL(0x0053414a, FUN_0053414a, "regs() -> (ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
