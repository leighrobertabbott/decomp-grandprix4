#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004ccd10 { uint8_t Unknown00[0x94]; uint8_t Flag94; };
#pragma pack(pop)

// Stores the low byte of its stack argument at +0x94 of the object.
// FUNCTION: GP4 0x004ccd10
void __fastcall FUN_004ccd10(Unknown_004ccd10* self, void* /*edx*/, uint8_t value) { self->Flag94 = value; }
GP4_IMPL(0x004ccd10, FUN_004ccd10, "thiscall(ptr[0xa0]:bytes, i32) -> void")
