#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00471ee0 { uint8_t Unknown00[4]; uint8_t* Buffer04; int32_t Length08; };
#pragma pack(pop)

// Sets the length at +8 and NUL-terminates the buffer at +4 at that position.
// FUNCTION: GP4 0x00471ee0
void __fastcall FUN_00471ee0(Unknown_00471ee0* self, void* /*edx*/, int32_t length) {
    self->Length08 = length;
    self->Buffer04[length] = 0;
}
GP4_IMPL(0x00471ee0, FUN_00471ee0, "thiscall(ptr[0x10]:layout=str_00471ee0, i32[0..255]) -> void")
