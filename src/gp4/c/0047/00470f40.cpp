#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_00470f40 {
    uint8_t Unknown00[0xd84];
    float Value0D84;
};
#pragma pack(pop)

// Stores the argument at +0xd84, clamped: values above the limit at 0x005f0770 become 1000.0, values
// below the limit at 0x005f04d8 become 0.1.
// FUNCTION: GP4 0x00470f40
void __fastcall FUN_00470f40(Object_00470f40* self, void* /*edx*/, float value) {
    if (value <= GP4_GLOBAL(float, 0x005f0770)) {
        if (value < GP4_GLOBAL(float, 0x005f04d8)) {
            self->Value0D84 = 0.1f;
            return;
        }
        if (value <= GP4_GLOBAL(float, 0x005f0770)) {
            self->Value0D84 = value;
            return;
        }
    }
    self->Value0D84 = 1000.0f;
}
GP4_IMPL(0x00470f40, FUN_00470f40, "thiscall(ptr[0xd90]:bytes, f32[-10..2000]) -> void")
