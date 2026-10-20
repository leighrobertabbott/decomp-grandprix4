#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns CF = 1 when the range [0x00963240, 0x00963244] has fewer than two entries (the end is
// below the start, or equal to it); otherwise calls 0x0055200d, then 0x00552045 / 0x00552046
// depending on [0x0096320c] being zero, and returns CF = 0. All registers are restored
// (pushal/popal). The flags are those of the last flag writer with CF replaced:
//   end < start        : `sub ecx, [0x963240]`                 | CF
//   end == start       : `cmp ecx, 1` (ecx = end - start + 1)  | CF
//   otherwise          : `cmp dword [0x96320c], 0`             & ~CF
// The `jb` after the call to 0x0055200d is never taken: ecx = diff + 1 is in 1..0x80000000.

// FUNCTION: GP4 0x0055200e
void __cdecl FUN_0055200e(gp4::Regs* r) {
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x00963244);
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x00963240);
    const uint32_t diff = end - start;
    if ((int32_t)diff < 0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(end, start) | gp4::CF);
        return;
    }
    const uint32_t count = diff + 1u;
    if (count == 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(count, 1u) | gp4::CF);
        return;
    }
    gp4::cdecl_<0x0055200d, void>();
    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x0096320c);
    const uint32_t arith = gp4::flags::sub32(mode, 0u);
    if (mode == 0u) {
        gp4::cdecl_<0x00552045, void>();
    } else {
        gp4::cdecl_<0x00552046, void>();
    }
    r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::CF);
}
GP4_IMPL(0x0055200e, FUN_0055200e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
