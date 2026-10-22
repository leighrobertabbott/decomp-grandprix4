#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb680 { uint8_t Unknown00[0x20]; int32_t Value20; };
#pragma pack(pop)

// Returns the dword at +0x20 of the object.
// FUNCTION: GP4 0x005cb680
int32_t __fastcall FUN_005cb680(Unknown_005cb680* self, void* /*edx*/) { return self->Value20; }
GP4_IMPL(0x005cb680, FUN_005cb680, "thiscall(ptr[0x30]:bytes) -> i32")
