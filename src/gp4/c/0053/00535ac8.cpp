#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t distance(uint32_t from, uint32_t base) {
    return from - base;
}

// Address inside the saved-state buffer [0x007a93e0]: the offset
// (0x007c3ae4 - 0x007ad890) + (0x0066d814 - 0x0066d7a4) added to the buffer base. Flags are those of
// the final `add eax, [0x7a93e0]`.
// FUNCTION: GP4 0x00535ac8
void __cdecl FUN_00535ac8(gp4::Regs* r) {
    const uint32_t offset = distance(0x007c3ae4u, 0x007ad890u) + distance(0x0066d814u, 0x0066d7a4u);
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x007a93e0);
    r->eax = offset + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, base));
}
GP4_IMPL(0x00535ac8, FUN_00535ac8, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
