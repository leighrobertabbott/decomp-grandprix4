#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4d70 {
    uint8_t Unknown00[0x44];
    uint16_t Value44;
    uint16_t Value46;
    uint16_t Value48;
    uint16_t Value4a;
    uint16_t Value4c;
    uint16_t Value4e;
};
#pragma pack(pop)

// Calls the empty routine 0x005ae990, copies the six 16-bit values at +0x44..+0x4e of the object to the
// six word pointers passed on the stack, returns 0.
// FUNCTION: GP4 0x004d4d70
int __fastcall FUN_004d4d70(Unknown_004d4d70* self, void* /*edx*/, uint16_t* out0, uint16_t* out1,
                            uint16_t* out2, uint16_t* out3, uint16_t* out4, uint16_t* out5) {
    gp4::cdecl_<0x005ae990, void>();
    *out0 = self->Value44;
    *out1 = self->Value46;
    *out2 = self->Value48;
    *out3 = self->Value4a;
    *out4 = self->Value4c;
    *out5 = self->Value4e;
    return 0;
}
GP4_IMPL(0x004d4d70, FUN_004d4d70, "thiscall(ptr[0x50]:bytes, ptr[4]:bytes, ptr[4]:bytes, ptr[4]:bytes, ptr[4]:bytes, ptr[4]:bytes, ptr[4]:bytes) -> i32")
