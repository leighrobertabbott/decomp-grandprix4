#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t offsetInBlock(uint32_t from, uint32_t base) {
    return from - base;
}

// Address of the byte field at 0x007c2ea7 inside the copy of the saved-state block 0x007ad890.. that
// lives in the buffer [0x007a93e0]: [0x007a93e0] + (0x007c2ea7 - 0x007ad890).
// Flags are those of the final `add eax, [0x7a93e0]`.
// FUNCTION: GP4 0x00535ab7
void __cdecl FUN_00535ab7(gp4::Regs* r) {
    const uint32_t offset = offsetInBlock(0x007c2ea7u, 0x007ad890u);
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x007a93e0);
    r->eax = offset + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, base));
}
GP4_IMPL(0x00535ab7, FUN_00535ab7, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
