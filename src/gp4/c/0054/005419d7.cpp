#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the mode byte 0x007ab020 with 1 and, when it is not 1, with 2; both outcomes return to the
// same `ret`, so only the flags of the last compare remain.
// FUNCTION: GP4 0x005419d7
void __cdecl FUN_005419d7(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab020);
    const uint32_t arith = (mode == 1u) ? gp4::flags::sub8(mode, 1u) : gp4::flags::sub8(mode, 2u);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005419d7, FUN_005419d7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
