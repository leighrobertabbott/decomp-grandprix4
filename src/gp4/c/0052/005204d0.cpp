#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: resets row slot 2 through FUN_00520584 (eax = 2), clears [0x679b0c], zeroes the
// byte range [ [0x7a8e28], [0x7a8e2c] ) (rep stosb, ecx = end - start) and clears [0x66d830].
// pushal/popal restore every GPR; the flags are those of `sub ecx, edi` (end - start).
// FUNCTION: GP4 0x005204d0
void __cdecl FUN_005204d0(gp4::Regs* r) {
    gp4::Regs c = {};
    c.eax = 2u;
    gp4::call_regs(0x00520584, c);

    GP4_GLOBAL(uint32_t, 0x00679b0c) = 0u;
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x007a8e2c);
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x007a8e28);
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(start));
    uint32_t count = end - start;
    for (; count != 0u; --count) {
        *cursor = 0;
        ++cursor;
    }
    GP4_GLOBAL(uint32_t, 0x0066d830) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(end, start));
}
GP4_IMPL(0x005204d0, FUN_005204d0, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584+fill_005204d0")
