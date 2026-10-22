#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_4596b0 {
    uint8_t Unknown00[0x18];
    int32_t Mode18;
    uint8_t Unknown1c[0x1f0 - 0x1c];
    uint8_t* Records1f0;   // array of 0x1cc-byte records
};
#pragma pack(pop)

// Sets the dword at +0x198 of record `index` (records are 0x1cc bytes apart) from the mode in
// +0x18: 5 by default, 0x15 for mode 1, 0x25 for mode 2.
// FUNCTION: GP4 0x004596b0
void __fastcall FUN_004596b0(Unknown_4596b0* self, void* /*edx*/, int index) {
    uint32_t value = 5;
    if (self->Mode18 == 1) {
        value = 0x15;
    } else if (self->Mode18 == 2) {
        value = 0x25;
    }
    *reinterpret_cast<uint32_t*>(self->Records1f0 + 0x198 + index * 0x1cc) = value;
}
GP4_IMPL(0x004596b0, FUN_004596b0, "thiscall(ptr[0x1f8]:layout=rec_004596b0, i32[0..7]) -> void")
