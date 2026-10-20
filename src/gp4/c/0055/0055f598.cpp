#include <gp4/gp4.h>

// Maps a four-character format code to a small id: ' 16L' (0x36314c20) -> 0x32,
// 'A16L' (0x36314c41) -> 0x33, ' 16R' (0x36315220) -> 0x14, 'A16R' (0x36315241)
// -> 0x15; any other value is returned unchanged.

// FUNCTION: GP4 0x0055f598
int __cdecl FUN_0055f598(int code) {
    if (code == 0x36314c20) return 0x32;
    if (code == 0x36314c41) return 0x33;
    if (code == 0x36315220) return 0x14;
    if (code == 0x36315241) return 0x15;
    return code;
}
GP4_IMPL(0x0055f598, FUN_0055f598, "cdecl(i32{0x36314c20,0x36314c41,0x36315220,0x36315241}) -> i32")
