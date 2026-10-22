#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004e8b30 {
    uint8_t Unknown00[4];
    int32_t Divisor04;
    uint8_t Unknown08[4];
    uint8_t* Buffer0c;
    int32_t Base10;
};
#pragma pack(pop)

// Clears the byte of the buffer at +0xc indexed by (value - base) / divisor, base at +0x10 and
// divisor at +4.
// FUNCTION: GP4 0x004e8b30
void __fastcall FUN_004e8b30(Unknown_004e8b30* self, void* /*edx*/, int value) {
    self->Buffer0c[(value - self->Base10) / self->Divisor04] = 0;
}
GP4_IMPL(0x004e8b30, FUN_004e8b30, "thiscall(ptr[0x20]:{4:i32{=1,2,4,8}, 0xc:ptr[0x100]:bytes, 0x10:i32{=0}}, i32[0..0xff]) -> void")
