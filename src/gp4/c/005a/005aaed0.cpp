#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005aaed0 { uint8_t Unknown00[0x60]; int32_t Value60; uint8_t Unknown64[0x9c]; };
#pragma pack(pop)

// Stores the argument at +0x60 of the object (1 when the argument is 0); returns 0.
// FUNCTION: GP4 0x005aaed0
int32_t __fastcall FUN_005aaed0(Unknown_005aaed0* self, void* /*edx*/, int32_t value) {
    if (value == 0) {
        value = 1;
    }
    self->Value60 = value;
    return 0;
}
GP4_IMPL(0x005aaed0, FUN_005aaed0, "thiscall(ptr[0x100]:bytes, i32{0,1,2,-5,0x7fffffff}) -> i32")
