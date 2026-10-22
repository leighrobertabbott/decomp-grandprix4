#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d3910 { uint8_t Unknown00[0x98]; int32_t Value98; };
#pragma pack(pop)

// Setter: stores its stack argument at +0x98.
// FUNCTION: GP4 0x004d3910
void __fastcall FUN_004d3910(Unknown_004d3910* self, void* /*edx*/, int32_t value) { self->Value98 = value; }
GP4_IMPL(0x004d3910, FUN_004d3910, "thiscall(ptr[0xa8]:bytes, i32) -> u32")
