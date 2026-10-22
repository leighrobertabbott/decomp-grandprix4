#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046fda0 { uint8_t Unknown00[0x54]; uint32_t Value54; };
#pragma pack(pop)

// Copies the byte global at 0x00a6e21c (zero-extended) into +0x54 and returns (value == 0).
// FUNCTION: GP4 0x0046fda0
int __fastcall FUN_0046fda0(Unknown_0046fda0* self, void* /*edx*/, int32_t /*unused stack arg*/) {
    const uint32_t value = GP4_GLOBAL(uint8_t, 0x00a6e21c);
    self->Value54 = value;
    return value == 0;
}
GP4_IMPL(0x0046fda0, FUN_0046fda0, "thiscall(ptr[0x60]:bytes, i32) -> i32; globals=fuzz:i32")
