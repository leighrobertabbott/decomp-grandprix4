#include <gp4/gp4.h>

// Picks a small selector code from three adjacent byte flags: 0x15 if the first is 1, else 0x1d if the
// second is 1, else 0x1e plus 0x47 when the third is not 1 (the `neg al; sbb eax, eax; and eax, 0x47`
// mask is all ones exactly when (flag3 - 1) is nonzero).
// FUNCTION: GP4 0x004ac290
int __cdecl FUN_004ac290() {
    if (GP4_GLOBAL(uint8_t, 0x00a64f24) == 1u) {
        return 0x15;
    }
    if (GP4_GLOBAL(uint8_t, 0x00a64f25) == 1u) {
        return 0x1d;
    }
    int mask = (GP4_GLOBAL(uint8_t, 0x00a64f26) != 1u) ? -1 : 0;
    return (mask & 0x47) + 0x1e;
}
GP4_IMPL(0x004ac290, FUN_004ac290, "cdecl() -> i32; globals=fuzz:i32")
