#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4b80 { uint8_t Unknown00[0x60]; uint32_t* Items60; uint8_t Capacity64; uint8_t Count65; };
#pragma pack(pop)

// Appends a nonzero value to the bounded dword list at +0x60 (count at +0x65, capacity at +0x64);
// returns 0 on success, 1 when the value is zero or the list is full.
// FUNCTION: GP4 0x004d4b80
int __fastcall FUN_004d4b80(Unknown_004d4b80* self, void* /*edx*/, int value) {
    gp4::cdecl_<0x005ae990, void>();
    if (value != 0 && self->Count65 < self->Capacity64) {
        self->Items60[self->Count65] = value;
        self->Count65 = self->Count65 + 1;
        return 0;
    }
    return 1;
}
GP4_IMPL(0x004d4b80, FUN_004d4b80, "thiscall(ptr[0x70]:u32{=0x00640302,0x00640203,0x00640000,0x00640305,0x00640a05}, i32{0,1,0x1234,0x7fffffff}) -> i32")
