#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a8310 { uint8_t Unknown00[0x190]; uint8_t Bytes190[0x80]; };
#pragma pack(pop)

// Stores a byte into the byte table at +0x190 of the object, indexed by a signed char; returns 0.
// FUNCTION: GP4 0x005a8310
int32_t __fastcall FUN_005a8310(Unknown_005a8310* self, void* /*edx*/, int8_t index, uint8_t value) {
    self->Bytes190[index] = value;
    return 0;
}
GP4_IMPL(0x005a8310, FUN_005a8310, "thiscall(ptr[0x230]:bytes, i32[-128..127], u32) -> i32")
