#include <gp4/gp4.h>

typedef void (__stdcall *OutputDebugStringA_t)(const char* text);

// Marks the Bink init as completed (0x00abc114 = 1), logs "Bink init completed\n", returns true.
// FUNCTION: GP4 0x00504760
uint8_t __cdecl FUN_00504760() {
    GP4_GLOBAL(int, 0x00abc114) = 1;
    GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0066b97c));
    return 1;
}
GP4_IMPL(0x00504760, FUN_00504760, "cdecl() -> u8; globals=fuzz:i32")
