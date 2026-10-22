#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ae260 { uint8_t Unknown00[0x274]; uint8_t Flag274; };
#pragma pack(pop)

// Sets the byte at +0x274 of the object to 1 and returns 0.
// FUNCTION: GP4 0x005ae260
int __fastcall FUN_005ae260(Unknown_005ae260* self, void* /*edx*/) { self->Flag274 = 1; return 0; }
GP4_IMPL(0x005ae260, FUN_005ae260, "thiscall(ptr[0x280]:bytes) -> i32")
