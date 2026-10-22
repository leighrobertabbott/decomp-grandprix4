#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046b3b0 {
    uint8_t Unknown00[0x10];
    int32_t Field10;
};
#pragma pack(pop)

// Getter: returns the dword at +0x10 of its stack argument.
// FUNCTION: GP4 0x0046b3b0
int __stdcall FUN_0046b3b0(Unknown_0046b3b0* obj) { return obj->Field10; }
GP4_IMPL(0x0046b3b0, FUN_0046b3b0, "stdcall(ptr[0x18]:bytes) -> i32")
