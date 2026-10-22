#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4cb0 { uint8_t Unknown00[0x10]; uint8_t Value10; };
#pragma pack(pop)

// Calls the empty routine 0x005ae990, copies the byte at +0x10 of the object to the byte
// pointed to by its stack argument, returns 0.
// FUNCTION: GP4 0x004d4cb0
int __fastcall FUN_004d4cb0(Unknown_004d4cb0* self, void* /*edx*/, uint8_t* dest) {
    gp4::cdecl_<0x005ae990, void>();
    *dest = self->Value10;
    return 0;
}
GP4_IMPL(0x004d4cb0, FUN_004d4cb0, "thiscall(ptr[0x18]:bytes, ptr[8]:bytes) -> i32")
