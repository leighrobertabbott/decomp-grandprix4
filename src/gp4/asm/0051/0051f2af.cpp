#include <gp4/gp4.h>
#include <gp4/flags.h>

// Driver-name lookup: eax = address of the 0x18-byte name entry for car number (al & 0x3f), 1-based,
// in the table starting at "michael schumacher" (0x007c2b3c); number 0 maps to entry 0.
// The flags are those of the final `add eax, 0x7c2b3c`.
// FUNCTION: GP4 0x0051f2af
void __cdecl FUN_0051f2af(gp4::Regs* r) {
    int32_t index = static_cast<int32_t>(r->eax & 0x3fu) - 1;
    if (index < 0) {
        index = 0;
    }
    const uint32_t offset = static_cast<uint32_t>(index) * 0x18u;
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x007c2b3c)));
    r->eax = offset + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, base));
}
GP4_IMPL(0x0051f2af, FUN_0051f2af, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
