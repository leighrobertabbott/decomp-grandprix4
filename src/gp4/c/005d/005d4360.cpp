#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d4360 {
    uint8_t Unknown00[0x64];
    int32_t Value64;
    int32_t Value68;
    uint8_t Unknown6c[0x18];
    int32_t Value84;
    int32_t Value88;
    uint8_t Unknown8c[0x54];
    uint8_t Member0e0[1];
};
#pragma pack(pop)

// Stores the pair (first, second) at +0x84/+0x88 of the object and, when the byte flag is set, also at
// +0x64/+0x68; then forwards the pair to the sub-object at +0xe0 (0x005da5c0).
// FUNCTION: GP4 0x005d4360
void __fastcall FUN_005d4360(Unknown_005d4360* self, void* /*edx*/, int32_t first, int32_t second, uint8_t both) {
    if (both != 0) {
        self->Value64 = first;
        self->Value68 = second;
    }
    self->Value84 = first;
    self->Value88 = second;
    gp4::thiscall_<0x005da5c0, void>(self->Member0e0, first, second);
}
GP4_IMPL(0x005d4360, FUN_005d4360, "thiscall(ptr[0x200]:bytes, i32, i32, u32{0,1,0x100,2}) -> void")
