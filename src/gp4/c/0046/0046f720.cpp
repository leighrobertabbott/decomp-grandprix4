#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046f720 { uint8_t Unknown00[0xc84]; float Table0c84[16]; };
#pragma pack(pop)

// Returns element `index` of the float table at +0xc84 of the object.
// FUNCTION: GP4 0x0046f720
float __fastcall FUN_0046f720(Unknown_0046f720* self, void* /*edx*/, int index) { return self->Table0c84[index]; }
GP4_IMPL(0x0046f720, FUN_0046f720, "thiscall(ptr[0xd00]:f32, i32[0..15]) -> f32")
