#include <gp4/gp4.h>
#include <gp4/flags.h>

// ORs the bytes at 0x007aafe0 and 0x007aafe1 and returns only the flags of that `or al, [mem]`
// (CF = OF = AF = 0); eax is saved and restored around it.
// FUNCTION: GP4 0x0054b0dc
void __cdecl FUN_0054b0dc(gp4::Regs* r) {
    const uint32_t a = GP4_GLOBAL(uint8_t, 0x007aafe0);
    const uint32_t b = GP4_GLOBAL(uint8_t, 0x007aafe1);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(a | b));
}
GP4_IMPL(0x0054b0dc, FUN_0054b0dc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
