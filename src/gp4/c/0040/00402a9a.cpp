#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00402a9a
void __cdecl FUN_00402a9a() {
    GP4_GLOBAL(int32_t, 0x00622bd0) = gp4::x87::fistp32(GP4_GLOBAL(double, 0x00622c98) * GP4_GLOBAL(double, 0x006f2be4));
    GP4_GLOBAL(int32_t, 0x00622bd4) = gp4::x87::fistp32(GP4_GLOBAL(double, 0x00622ca0) * GP4_GLOBAL(double, 0x006f2be4));
    GP4_GLOBAL(int32_t, 0x00622bd8) = gp4::x87::fistp32(GP4_GLOBAL(double, 0x00622ca8) * GP4_GLOBAL(double, 0x006f2be4));
}
GP4_IMPL(0x00402a9a, FUN_00402a9a, "cdecl() -> void; globals=fuzz:f64")
