#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044c970 { uint8_t Unknown00[0x24b9]; uint8_t Flag24b9; };
#pragma pack(pop)

// Sets the byte flag at +0x24b9 of the object.
// FUNCTION: GP4 0x0044c970
void __fastcall FUN_0044c970(Unknown_0044c970* self, void* /*edx*/) { self->Flag24b9 = 1; }
GP4_IMPL(0x0044c970, FUN_0044c970, "thiscall(ptr[0x24d0]:bytes) -> void")
