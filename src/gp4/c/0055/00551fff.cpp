#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests the dword at 0x00876f7c against 0x68. When equal the routine continues with `or eax, eax`,
// so the flags are those of that instruction; otherwise they are those of the `cmp`.
// FUNCTION: GP4 0x00551fff
void __cdecl FUN_00551fff(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00876f7c);
    uint32_t flags = gp4::flags::sub32(value, 0x68u);
    if (value == 0x68u) {
        flags = gp4::flags::logic32(r->eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551fff, FUN_00551fff, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
