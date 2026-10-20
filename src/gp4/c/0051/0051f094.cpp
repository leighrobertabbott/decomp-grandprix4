#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051f094
// Tail wrapper: `call 0x00481580 ; ret`. The callee is compiled C that reads no register: it clears
// the six flag bytes at 0x00a4b430.. of the HUD record selected by [0x00a4df50] and sets the one at
// +2 of the record for the current player. The caller (0x00445e86) does not read eax or the flags
// afterwards.
void __cdecl FUN_0051f094() {
    gp4::cdecl_<0x00481580, void>();
}
GP4_IMPL(0x0051f094, FUN_0051f094, "cdecl() -> void; globals=fuzz:i32+layout=view_00a4df50")
