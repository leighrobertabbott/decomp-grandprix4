#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x004028cf
void __cdecl FUN_004028cf(void) {
    GP4_GLOBAL(double, 0x00622cb0) = (double)GP4_GLOBAL(int32_t, 0x00623eb8) * GP4_GLOBAL(double, 0x006f2bdc);
    GP4_GLOBAL(double, 0x00622cb8) = (double)GP4_GLOBAL(int32_t, 0x00623ebc) * GP4_GLOBAL(double, 0x006f2bdc);
    GP4_GLOBAL(double, 0x00622cc0) = (double)GP4_GLOBAL(int32_t, 0x00623ec0) * GP4_GLOBAL(double, 0x006f2bdc);
}
GP4_IMPL(0x004028cf, FUN_004028cf, "cdecl() -> void")
