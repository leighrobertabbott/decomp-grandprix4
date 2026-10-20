#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the current directory through 0x005368c3 to one of two root strings: "D:\grand prix 3"
// (0x0081e5d9) when al is greater (signed byte compare) than the low byte of 0x008339ae, otherwise
// "C:\" (0x0081e6ea). eax is the BOOL result and the flags are those 0x005368c3 leaves.
// FUNCTION: GP4 0x005367b9
void __cdecl FUN_005367b9(gp4::Regs* r) {
    const int8_t selector = static_cast<int8_t>(r->eax);
    const int8_t limit = static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x008339ae));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    if (selector > limit) {
        c.eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x0081e5d9)));
    } else {
        c.eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x0081e6ea)));
    }
    gp4::call_regs(0x005368c3, c);
    r->eax = c.eax;
    r->eflags = c.eflags;
}
GP4_IMPL(0x005367b9, FUN_005367b9, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
