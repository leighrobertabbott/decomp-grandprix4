#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bc220 { uint8_t Unknown00[0x24]; uint8_t Member24[1]; };
#pragma pack(pop)

// Returns the address of the sub-object at +0x24 (lea eax, [ecx+0x24]).
// FUNCTION: GP4 0x005bc220
uint8_t* __fastcall FUN_005bc220(Unknown_005bc220* self, void* /*edx*/) { return self->Member24; }
GP4_IMPL(0x005bc220, FUN_005bc220, "thiscall(ptr[0x30]:bytes) -> i32")
