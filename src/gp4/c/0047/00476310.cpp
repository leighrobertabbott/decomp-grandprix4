#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00476310 {
    uint8_t Unknown00[0x10];
    int32_t Field10;
};
#pragma pack(pop)

// Getter: returns the dword at this+0x10.
// FUNCTION: GP4 0x00476310
int __fastcall FUN_00476310(Unknown_00476310* self, void* /*edx*/) { return self->Field10; }
GP4_IMPL(0x00476310, FUN_00476310, "thiscall(ptr[0x18]:bytes) -> i32")
