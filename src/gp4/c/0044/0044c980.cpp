#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044c980 { uint8_t Unknown00[0x24b9]; uint8_t Flag24b9; };
#pragma pack(pop)

// Clears the byte flag at +0x24b9 of the object.
// FUNCTION: GP4 0x0044c980
void __fastcall FUN_0044c980(Unknown_0044c980* self, void* /*edx*/) { self->Flag24b9 = 0; }
GP4_IMPL(0x0044c980, FUN_0044c980, "thiscall(ptr[0x24d0]:bytes) -> void")
