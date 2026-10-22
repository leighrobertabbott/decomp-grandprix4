#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_0047ddb0 { uint8_t Unknown00[0xb8]; float ValueB8; };
#pragma pack(pop)

// Adds the float constant at 0x005f0880 to the float at +0xb8 of the object.
// FUNCTION: GP4 0x0047ddb0
void __fastcall FUN_0047ddb0(Unknown_0047ddb0* self, void* /*edx*/) {
    self->ValueB8 = self->ValueB8 + GP4_GLOBAL(float, 0x005f0880);
}
GP4_IMPL(0x0047ddb0, FUN_0047ddb0, "thiscall(ptr[0xc0]:f32) -> void")
