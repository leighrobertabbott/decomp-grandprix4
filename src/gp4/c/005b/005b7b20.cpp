#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b7b20 { uint8_t Unknown00[0x54]; int32_t Value54; };
#pragma pack(pop)

// Returns the dword at +0x54 of the node in slot `index` of the pointer table at 0x00b35afc.
// FUNCTION: GP4 0x005b7b20
int32_t __stdcall FUN_005b7b20(int index) {
    Unknown_005b7b20* node = GP4_ARRAY(Unknown_005b7b20*, 0x00b35afc)[index];
    return node->Value54;
}
GP4_IMPL(0x005b7b20, FUN_005b7b20, "stdcall(i32[0..15]) -> i32; gfields={0x00b35afc*16/4:ptr[0x100]:bytes}")
