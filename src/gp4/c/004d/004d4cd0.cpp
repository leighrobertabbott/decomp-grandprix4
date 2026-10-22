#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4cd0 { uint8_t Unknown00[0x38]; uint32_t Value38; uint32_t Value3c; };
#pragma pack(pop)

// Calls the empty routine 0x005ae990, copies the dwords at +0x38 and +0x3c of the object to the two
// dwords pointed to by its stack arguments, returns 0.
// FUNCTION: GP4 0x004d4cd0
int __fastcall FUN_004d4cd0(Unknown_004d4cd0* self, void* /*edx*/, uint32_t* out38, uint32_t* out3c) {
    gp4::cdecl_<0x005ae990, void>();
    *out38 = self->Value38;
    *out3c = self->Value3c;
    return 0;
}
GP4_IMPL(0x004d4cd0, FUN_004d4cd0, "thiscall(ptr[0x48]:bytes, ptr[8]:bytes, ptr[8]:bytes) -> i32")
