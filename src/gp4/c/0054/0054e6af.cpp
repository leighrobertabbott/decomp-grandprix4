#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054e6af
// High-resolution timer fraction. Reads the performance counter into the qword at 0x00838c5c
// (thunk 0x005a48be: jmp [0x5f0254], kernel32!QueryPerformanceCounter), subtracts the start
// qword at 0x00838c54, scales the 64-bit difference by 2^16 and divides the upper 64 bits of
// that by the counter frequency dword 0x00838c4c: eax = ((delta << 16) / frequency), or
// 0x40000000 when the quotient would overflow 32 bits (frequency <= delta >> 16). ebx, ecx and
// edx are saved and restored around the call; the flags are those of `cmp ebx, edx`.
void __cdecl FUN_0054e6af(gp4::Regs* r) {
    gp4::stdcall_<0x005a48be, int>(&GP4_GLOBAL(uint32_t, 0x00838c5c));

    uint32_t low = GP4_GLOBAL(uint32_t, 0x00838c5c);
    uint32_t high = GP4_GLOBAL(uint32_t, 0x00838c60);
    const uint32_t startLow = GP4_GLOBAL(uint32_t, 0x00838c54);
    const uint32_t startHigh = GP4_GLOBAL(uint32_t, 0x00838c58);

    const uint32_t borrow = (low < startLow) ? 1u : 0u;
    low = low - startLow;
    high = high - startHigh - borrow;

    const uint32_t upper = (high << 16) | (low >> 16);
    const uint32_t lower = low << 16;
    const uint32_t frequency = GP4_GLOBAL(uint32_t, 0x00838c4c);

    uint32_t result;
    if (frequency <= upper) {
        result = 0x40000000u;
    } else {
        uint32_t remainder;
        result = gp4::div64((static_cast<uint64_t>(upper) << 32) | lower, frequency, &remainder);
    }

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(frequency, upper));
}
GP4_IMPL(0x0054e6af, FUN_0054e6af, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
