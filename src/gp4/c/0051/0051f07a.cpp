#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051f07a
// Tail wrapper: `call 0x00481520 ; ret`. The callee is compiled C that reads no register: it clears
// the six flag bytes at 0x00a4b430.. of the HUD record selected by [0x00a4df50] and sets the one at
// +1 of the record for the current player. The callers (0x004425e0, 0x00445e86) do not read eax or
// the flags afterwards.
void __cdecl FUN_0051f07a() {
    gp4::cdecl_<0x00481520, void>();
}
GP4_IMPL(0x0051f07a, FUN_0051f07a, "cdecl() -> void; globals=fuzz:i32+layout=view_00a4df50")
