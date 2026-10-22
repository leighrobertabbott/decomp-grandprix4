#include <gp4/gp4.h>

// Copies "is nonzero" of the dword global 0x00a69878 into the byte flag 0x00a562ef.
// FUNCTION: GP4 0x00499f70
void __cdecl FUN_00499f70() {
    GP4_GLOBAL(uint8_t, 0x00a562ef) = GP4_GLOBAL(uint32_t, 0x00a69878) != 0;
}
GP4_IMPL(0x00499f70, FUN_00499f70, "cdecl() -> void; globals=fuzz:i32")
