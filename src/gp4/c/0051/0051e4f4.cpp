#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051e4f4
// call 0x51e4fb (which itself never returns), then spin on `jmp $`.
void __cdecl FUN_0051e4f4() {
    gp4::cdecl_<0x0051e4fb, void>();
    for (;;) {
    }
}
GP4_IMPL(0x0051e4f4, FUN_0051e4f4, "cdecl() -> void")
