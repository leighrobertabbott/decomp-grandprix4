#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4d50 { uint8_t Unknown00[0x42]; uint8_t Value42; };
#pragma pack(pop)

// Calls the empty routine 0x005ae990, copies the byte at +0x42 of the object to the byte
// pointed to by its stack argument, returns 0.
// FUNCTION: GP4 0x004d4d50
int __fastcall FUN_004d4d50(Unknown_004d4d50* self, void* /*edx*/, uint8_t* dest) {
    gp4::cdecl_<0x005ae990, void>();
    *dest = self->Value42;
    return 0;
}
GP4_IMPL(0x004d4d50, FUN_004d4d50, "thiscall(ptr[0x50]:bytes, ptr[8]:bytes) -> i32")
