#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the dword at 0x007a979c is zero, increments the (unaligned) dword at 0x00828293. The flags are
// those of `test dword [0x7a979c], -1`, or of the `add dword [0x828293], 1` when it ran.
// FUNCTION: GP4 0x00534c86
void __cdecl FUN_00534c86(gp4::Regs* r) {
    const uint32_t gate = GP4_GLOBAL(uint32_t, 0x007a979c);
    uint32_t flags = gp4::flags::logic32(gate);
    if (gate == 0u) {
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
        GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;
        flags = gp4::flags::add32(before, 1u);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00534c86, FUN_00534c86, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
