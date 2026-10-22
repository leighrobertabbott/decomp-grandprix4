#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ae250 { uint8_t Unknown00[0x274]; uint8_t Flag274; };
#pragma pack(pop)

// Clears the byte at +0x274 of the object and returns 0.
// FUNCTION: GP4 0x005ae250
int __fastcall FUN_005ae250(Unknown_005ae250* self, void* /*edx*/) { self->Flag274 = 0; return 0; }
GP4_IMPL(0x005ae250, FUN_005ae250, "thiscall(ptr[0x280]:bytes) -> i32")
