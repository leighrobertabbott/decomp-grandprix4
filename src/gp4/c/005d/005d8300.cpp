#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d8300 { uint8_t Unknown00[0x710]; int32_t Value710; uint8_t Unknown714[8]; };
#pragma pack(pop)

// True unless the dword at +0x710 of the object is 5 (`cmp [ecx+0x710], 5; setne al; ret`).
// FUNCTION: GP4 0x005d8300
uint8_t __fastcall FUN_005d8300(const Unknown_005d8300* self, void* /*edx*/) {
    return self->Value710 != 5;
}
GP4_IMPL(0x005d8300, FUN_005d8300, "thiscall(ptr[0x718]:u32{0,1,2,3,4,5,5,6}) -> u8")
