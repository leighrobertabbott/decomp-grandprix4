#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d7fe0 { uint8_t Unknown00[0x72c]; int32_t Value72c; };
#pragma pack(pop)

// Returns the dword at +0x72c of the object; ignores its stack argument (`ret 4`).
// FUNCTION: GP4 0x005d7fe0
int __fastcall FUN_005d7fe0(Unknown_005d7fe0* self, void* /*edx*/, int32_t /*unused*/) {
    return self->Value72c;
}
GP4_IMPL(0x005d7fe0, FUN_005d7fe0, "thiscall(ptr[0x740]:bytes, i32) -> i32")
