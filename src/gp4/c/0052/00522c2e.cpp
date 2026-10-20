#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills the two 22-dword tables 0x007c312c and 0x007c3184 with 0x20000000. ebx ends at 0x58 (byte
// offset after the last pair), ecx at 0; the flags are those of the last `add ebx, 4` (0x54 + 4).
// FUNCTION: GP4 0x00522c2e
void __cdecl FUN_00522c2e(gp4::Regs* r) {
    uint32_t* first = GP4_ARRAY(uint32_t, 0x007c312c);
    uint32_t* second = GP4_ARRAY(uint32_t, 0x007c3184);
    for (uint32_t i = 0; i != 0x16u; ++i) {
        first[i] = 0x20000000u;
        second[i] = 0x20000000u;
    }
    r->ebx = 0x58u;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0x54u, 4u));
}
GP4_IMPL(0x00522c2e, FUN_00522c2e, "regs() -> (ebx, ecx, cf, pf, af, zf, sf, of)")
