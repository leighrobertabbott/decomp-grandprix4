#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the low word 0x4210 into [0x827559] (0x531af4), then calls the empty stubs 0x531b29 (twice) and
// 0x531b27 (five times) with ebx = [0x81d1c8] + 0x28, eax = [0x81d1cc] + 0x10, ecx = ebx + 0xd0 and a
// countdown in edx; eax advances by 0x19 per call. Exit: eax = [0x81d1cc] + 0x10 + 5 * 0x19,
// ebx = [0x81d1c8] + 0x28, ecx = ebx + 0xd0, edx = 0; the flags are those of the last `dec edx` (CF kept
// from the last `add eax, 0x19`).
// FUNCTION: GP4 0x0052a78d
void __cdecl FUN_0052a78d(gp4::Regs* r) {
    gp4::Regs c = {};
    c.eax = 0x4210u;
    gp4::call_regs(0x00531af4, c);

    gp4::cdecl_<0x00531b29, void>();
    gp4::cdecl_<0x00531b29, void>();

    const uint32_t x = GP4_GLOBAL(uint32_t, 0x0081d1c8) + 0x28u;
    uint32_t y = GP4_GLOBAL(uint32_t, 0x0081d1cc) + 0x10u;
    uint32_t arith = 0u;
    uint32_t counter = 5u;
    do {
        gp4::cdecl_<0x00531b27, void>();
        arith = gp4::flags::add32(y, 0x19u);
        y += 0x19u;
        --counter;
    } while (counter != 0u);

    r->eax = y;
    r->ebx = x;
    r->ecx = x + 0xd0u;
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, arith));
}
GP4_IMPL(0x0052a78d, FUN_0052a78d, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
