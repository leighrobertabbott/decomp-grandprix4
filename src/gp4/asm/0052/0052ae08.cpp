#include <gp4/gp4.h>

// FUNCTION: GP4 0x0052ae08
// Splits a millisecond count in eax into hours / minutes / seconds / milliseconds:
// 0x8236c0 = ms % 1000, 0x8236bc = seconds % 60, 0x8236b8 = minutes % 60, 0x8236b4 = hours.
// The `cdq; div` pairs treat eax as non-negative (callers guard eax < 0x3e80000; a value with the
// sign bit set would overflow the 64/32 divide and fault). pushal/popal preserve every register
// and the flags are untouched (div leaves them as they were).
void __cdecl FUN_0052ae08(gp4::Regs* r) {
    const uint32_t millis = r->eax;

    const uint32_t totalSeconds = millis / 1000u;
    const uint32_t ms = millis % 1000u;
    const uint32_t totalMinutes = totalSeconds / 60u;
    const uint32_t seconds = totalSeconds % 60u;
    const uint32_t hours = totalMinutes / 60u;
    const uint32_t minutes = totalMinutes % 60u;

    GP4_GLOBAL(uint32_t, 0x008236c0) = ms;
    GP4_GLOBAL(uint32_t, 0x008236bc) = seconds;
    GP4_GLOBAL(uint32_t, 0x008236b8) = minutes;
    GP4_GLOBAL(uint32_t, 0x008236b4) = hours;
}
GP4_IMPL(0x0052ae08, FUN_0052ae08, "regs(eax:u32[0..0x3e7ffff]) -> (); globals=fuzz:i32")
