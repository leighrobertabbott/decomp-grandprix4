#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_0047ddd0 { uint8_t Unknown00[0xbc]; float ValueBC; };
#pragma pack(pop)

// Adds the float constant at 0x005f0880 to the float at +0xbc of the object.
// FUNCTION: GP4 0x0047ddd0
void __fastcall FUN_0047ddd0(Unknown_0047ddd0* self, void* /*edx*/) {
    self->ValueBC = self->ValueBC + GP4_GLOBAL(float, 0x005f0880);
}
GP4_IMPL(0x0047ddd0, FUN_0047ddd0, "thiscall(ptr[0xc4]:f32) -> void")
