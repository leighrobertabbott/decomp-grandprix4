#include <gp4/gp4.h>

// `call 0x005ae990 ; call 0x005ae990 ; ret`: calls the empty routine at 0x005ae990 twice.
// FUNCTION: GP4 0x00533801
void __cdecl FUN_00533801() {
    gp4::cdecl_<0x005ae990, void>();
    gp4::cdecl_<0x005ae990, void>();
}
GP4_IMPL(0x00533801, FUN_00533801, "cdecl() -> void")
