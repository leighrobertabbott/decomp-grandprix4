#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte length of [start, end): kept out of line so MSVC does not fold the two addresses into one constant (the
// original subtracts them at run time).
static __declspec(noinline) uint32_t Span(uint32_t end, uint32_t start) {
    return end - start;
}

// pushal wrapper: when the one-shot flag byte [0x0085306c] is nonzero, copies the 0xa41 bytes at 0x0085306d back to
// 0x007c30a3 (the block the saver 0x00520cf4 stashed) and clears the flag. popal restores every register; the flags
// are those of `cmp byte [0x0085306c], 0` when the flag is clear, else those of the `sub ecx, 0x7c30a3` that
// builds the byte count.
// FUNCTION: GP4 0x00520d28
void __cdecl FUN_00520d28(gp4::Regs* r) {
    const uint8_t pending = GP4_GLOBAL(uint8_t, 0x0085306c);
    uint32_t arith = gp4::flags::sub8(pending, 0u);
    if (pending != 0u) {
        arith = gp4::flags::sub32(0x007c3ae4u, 0x007c30a3u);
        uint32_t count = Span(0x007c3ae4u, 0x007c30a3u);
        uint32_t source = 0x0085306du;
        uint32_t target = 0x007c30a3u;
        while (count != 0u) {
            GP4_GLOBAL(uint8_t, target) = GP4_GLOBAL(uint8_t, source);
            source += 1u;
            target += 1u;
            count -= 1u;
        }
        GP4_GLOBAL(uint8_t, 0x0085306c) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00520d28, FUN_00520d28, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=once_00520cf4+stash_0085306d")
