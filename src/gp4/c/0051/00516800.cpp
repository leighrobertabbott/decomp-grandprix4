#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of the driver-name entry; kept out of line so the table base 0x007c2b3c stays visible as the operand of
// the original `add eax, 0x7c2b3c`.
static __declspec(noinline) uint32_t entryAddr(uint32_t scaledIndex, uint32_t base) {
    return scaledIndex + base;
}

// pushal/popal routine: index = min(max(([0x00675958] & 0x3f) - 1, 0), 0x15); [0x00a6987c] = 0x007c2b3c + 0x18 *
// index (the address of that driver's 0x18-byte name entry). Every GPR is restored; the flags are those of the
// final `add eax, 0x7c2b3c`.
// FUNCTION: GP4 0x00516800
void __cdecl FUN_00516800(gp4::Regs* r) {
    int32_t index = static_cast<int32_t>(GP4_GLOBAL(uint8_t, 0x00675958) & 0x3fu) - 1;
    if (index < 0) {
        index = 0;
    }
    if (index > 0x15) {
        index = 0x15;
    }
    const uint32_t scaled = static_cast<uint32_t>(index) * 0x18u;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = entryAddr(scaled, 0x007c2b3cu);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(scaled, 0x007c2b3cu));
}
GP4_IMPL(0x00516800, FUN_00516800, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
