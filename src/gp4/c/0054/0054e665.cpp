#include <gp4/gp4.h>
#include <gp4/flags.h>

// High-resolution timer fraction (sibling of 0x0054e6af, scale 2^8 instead of 2^16). Reads the
// performance counter into the qword at 0x00838c5c (thunk 0x005a48be: jmp [0x5f0254],
// kernel32!QueryPerformanceCounter), subtracts the start qword at 0x00838c54, shifts the 64-bit
// difference left by 8 and divides it by the counter frequency dword 0x00838c4c:
// eax = (delta << 8) / frequency, or eax = frequency itself when the upper half of the shifted
// delta is >= frequency (the quotient would overflow 32 bits). ebx, ecx and edx are saved on
// entry and restored on exit, so the remainder never leaves; the flags are those of
// `cmp ebx, edx` (frequency against the upper half) - div and the pops leave them alone.
// FUNCTION: GP4 0x0054e665
void __cdecl FUN_0054e665(gp4::Regs* r) {
    gp4::stdcall_<0x005a48be, int>(&GP4_GLOBAL(uint32_t, 0x00838c5c));

    uint32_t low = GP4_GLOBAL(uint32_t, 0x00838c5c);
    uint32_t high = GP4_GLOBAL(uint32_t, 0x00838c60);
    const uint32_t startLow = GP4_GLOBAL(uint32_t, 0x00838c54);
    const uint32_t startHigh = GP4_GLOBAL(uint32_t, 0x00838c58);

    const uint32_t borrow = (low < startLow) ? 1u : 0u;
    low = low - startLow;
    high = high - startHigh - borrow;

    const uint32_t upper = (high << 8) | (low >> 24);
    const uint32_t lower = low << 8;
    const uint32_t frequency = GP4_GLOBAL(uint32_t, 0x00838c4c);

    uint32_t result;
    if (frequency <= upper) {
        result = frequency;
    } else {
        uint32_t remainder;
        result = gp4::div64((static_cast<uint64_t>(upper) << 32) | lower, frequency, &remainder);
    }

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(frequency, upper));
}
GP4_IMPL(0x0054e665, FUN_0054e665, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
