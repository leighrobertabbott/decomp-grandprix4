#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004cb030 {
    uint8_t Unknown00[4];
    int32_t Field04;
};
#pragma pack(pop)

// Setter: stores its stack argument at this+4.
// FUNCTION: GP4 0x004cb030
void __fastcall FUN_004cb030(Unknown_004cb030* self, void* /*edx*/, int value) {
    self->Field04 = value;
}
GP4_IMPL(0x004cb030, FUN_004cb030, "thiscall(ptr[0x0c]:bytes, i32) -> void")
