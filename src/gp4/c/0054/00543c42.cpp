#include <gp4/gp4.h>
#include <gp4/flags.h>

// Derives six values from [0xb1bf98] (a size) and [0xb1bf1c] / [0xb1bf40] (two lengths):
//   [0x83153b] = size * 5 / 0x30            [0x83153f] = size * 0x2c / 0x30     (signed)
//   [0x83154f] = [0x83153f] - [0x83153b]    [0x831547] = [0xb1bf1c] * 12
//   [0x83154b] = [0xb1bf40] - [0xb1bf1c]    [0x831543] = [0x83154b] - [0x831547]
// Exit: eax = [0x831543], ebx = 0x30, edx = the remainder of the second division; the flags are those
// of the final `sub eax, [0x831547]`.
// FUNCTION: GP4 0x00543c42
void __cdecl FUN_00543c42(gp4::Regs* r) {
    const int32_t first = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00b1bf98) * 5u);
    GP4_GLOBAL(int32_t, 0x0083153b) = first / 0x30;

    const int32_t second = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00b1bf98) * 0x2cu);
    const int32_t quotient = second / 0x30;
    const int32_t remainder = second % 0x30;
    GP4_GLOBAL(int32_t, 0x0083153f) = quotient;
    GP4_GLOBAL(int32_t, 0x0083154f) = quotient - GP4_GLOBAL(int32_t, 0x0083153b);

    GP4_GLOBAL(uint32_t, 0x00831547) = GP4_GLOBAL(uint32_t, 0x00b1bf1c) * 0xcu;

    const uint32_t gap = GP4_GLOBAL(uint32_t, 0x00b1bf40) - GP4_GLOBAL(uint32_t, 0x00b1bf1c);
    GP4_GLOBAL(uint32_t, 0x0083154b) = gap;
    const uint32_t result = gap - GP4_GLOBAL(uint32_t, 0x00831547);
    GP4_GLOBAL(uint32_t, 0x00831543) = result;

    r->eax = result;
    r->ebx = 0x30u;
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(gap, GP4_GLOBAL(uint32_t, 0x00831547)));
}
GP4_IMPL(0x00543c42, FUN_00543c42, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
