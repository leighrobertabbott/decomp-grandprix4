#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00470110 {
    uint8_t Unknown00[0xc94];
    int32_t Value0c94;
};
#pragma pack(pop)

// Stores the argument at +0xc94, resetting it to 0 when it exceeds 10.
// FUNCTION: GP4 0x00470110
void __fastcall FUN_00470110(Unknown_00470110* self, void* /*edx*/, int32_t value) {
    self->Value0c94 = value;
    if (value > 10) {
        self->Value0c94 = 0;
    }
}
GP4_IMPL(0x00470110, FUN_00470110, "thiscall(ptr[0xca0]:bytes, i32[-5..20]) -> void")
