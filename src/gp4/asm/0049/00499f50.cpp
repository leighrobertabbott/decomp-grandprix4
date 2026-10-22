#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Block_00a532b4 { uint32_t Words[6]; };
#pragma pack(pop)

// Copies the six-dword block at 0x00a532b4 to the destination pointer.
// FUNCTION: GP4 0x00499f50
void __cdecl FUN_00499f50(Block_00a532b4* dest) {
    *dest = GP4_GLOBAL(Block_00a532b4, 0x00a532b4);
}
GP4_IMPL(0x00499f50, FUN_00499f50, "cdecl(ptr[0x20]:bytes) -> void; globals=fuzz:i32")
