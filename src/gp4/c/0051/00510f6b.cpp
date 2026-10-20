#include <gp4/gp4.h>
#include <gp4/flags.h>

// "test dword [0x007a9780], -1; je ret; mov ax, 0x4a; call 0x00531f9c; ret": runs dispatcher command
// 0x4a only when the dword at 0x007a9780 is nonzero (the flags are then the dispatcher's, otherwise
// those of the test). The upper half of eax is untouched.
// FUNCTION: GP4 0x00510f6b
void __cdecl FUN_00510f6b(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x007a9780);
    if (value == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
        return;
    }
    r->eax = (r->eax & 0xffff0000u) | 0x4au;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x00510f6b, FUN_00510f6b, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
