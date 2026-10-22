#include <gp4/gp4.h>

// Stores the mouse-pointer-style value [0x00a58ad8] into field +0xb0 of object 14 of the table read by
// 0x0045ee70 when the global byte [0x00a4d5e8] and the argument are both nonzero, else -1.
// FUNCTION: GP4 0x004a4660
void __cdecl FUN_004a4660(char enable) {
    uint8_t* object = gp4::cdecl_<0x0045ee70, uint8_t*>(0xe);
    if (GP4_GLOBAL(uint8_t, 0x00a4d5e8) != 0 && enable != 0) {
        GP4_FIELD(uint32_t, object, 0xb0) = GP4_GLOBAL(uint32_t, 0x00a58ad8);
        return;
    }
    GP4_FIELD(uint32_t, object, 0xb0) = 0xffffffffu;
}
GP4_IMPL(0x004a4660, FUN_004a4660, "cdecl(i8) -> void; globals=layout=obj_004a4660")
