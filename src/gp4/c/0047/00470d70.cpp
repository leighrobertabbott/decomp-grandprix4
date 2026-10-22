#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00470d70 { uint8_t Unknown00[0xc9c]; uint32_t ValueC9C; };
#pragma pack(pop)

// Returns the dword at +0xc9c of the object.
// FUNCTION: GP4 0x00470d70
uint32_t __fastcall FUN_00470d70(Unknown_00470d70* self, void* /*edx*/) { return self->ValueC9C; }
GP4_IMPL(0x00470d70, FUN_00470d70, "thiscall(ptr[0xca8]:bytes) -> u32")
