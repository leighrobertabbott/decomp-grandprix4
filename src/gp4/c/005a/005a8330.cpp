#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a8330 { uint8_t Unknown00[0x190]; uint8_t Bytes190[0x80]; };
#pragma pack(pop)

// Reads the byte at +0x190 + index (index is a signed char) into the byte the second argument
// points at; returns 0.
// FUNCTION: GP4 0x005a8330
int32_t __fastcall FUN_005a8330(Unknown_005a8330* self, void* /*edx*/, int8_t index, uint8_t* out) {
    *out = self->Bytes190[index];
    return 0;
}
GP4_IMPL(0x005a8330, FUN_005a8330, "thiscall(ptr[0x230]:bytes, i32[-128..127], ptr[16]:bytes) -> i32")
