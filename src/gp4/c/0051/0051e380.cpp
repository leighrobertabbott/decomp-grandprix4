#include <gp4/gp4.h>

// Force-feedback update for the two effect slots. Slot 0 (when the enable byte [0x007a97a0] is set)
// copies the force [0x006f2d54] to [0x006f2d50], taking its magnitude when the byte [0x006f2d6c] is
// zero and the force is negative, and passes (id [0x006f2d4c], [0x006f2d50], [0x006f2d54],
// [0x006f2d58]) to the effect routine 0x005069d0. Slot 1 (enable [0x007a97a1]) does the same with
// [0x006f2d5c], [0x006f2d60], [0x006f2d64] and [0x006f2d68].
// FUNCTION: GP4 0x0051e380
void __cdecl FUN_0051e380(void) {
    if (GP4_GLOBAL(uint8_t, 0x007a97a0) != 0u) {
        int32_t force = GP4_GLOBAL(int32_t, 0x006f2d54);
        if (GP4_GLOBAL(uint8_t, 0x006f2d6c) == 0u && force < 0) {
            force = -force;
        }
        GP4_GLOBAL(int32_t, 0x006f2d50) = force;
        gp4::stdcall_<0x005069d0, void>(GP4_GLOBAL(uint32_t, 0x006f2d4c),
                                        GP4_GLOBAL(uint32_t, 0x006f2d50),
                                        GP4_GLOBAL(uint32_t, 0x006f2d54),
                                        GP4_GLOBAL(uint32_t, 0x006f2d58));
    }
    if (GP4_GLOBAL(uint8_t, 0x007a97a1) != 0u) {
        int32_t force = GP4_GLOBAL(int32_t, 0x006f2d64);
        if (GP4_GLOBAL(uint8_t, 0x006f2d6c) == 0u && force < 0) {
            force = -force;
        }
        GP4_GLOBAL(int32_t, 0x006f2d60) = force;
        gp4::stdcall_<0x005069d0, void>(GP4_GLOBAL(uint32_t, 0x006f2d5c),
                                        GP4_GLOBAL(uint32_t, 0x006f2d60),
                                        GP4_GLOBAL(uint32_t, 0x006f2d64),
                                        GP4_GLOBAL(uint32_t, 0x006f2d68));
    }
}
GP4_IMPL(0x0051e380, FUN_0051e380, "cdecl() -> void; globals=layout=cf_0041a2b2+ffb_0051e380")
