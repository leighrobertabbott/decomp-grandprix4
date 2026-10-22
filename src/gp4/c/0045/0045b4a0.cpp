#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045b4a0_Table { uint8_t Unknown00[0x14]; uint8_t* Records14; };
struct Unknown_0045b4a0 { uint8_t Unknown00[8]; Unknown_0045b4a0_Table* Table08; int32_t Index0c; };
#pragma pack(pop)

// Stores its stack argument in the first dword of record `Index0c` (stride 0x24) of the
// table whose base pointer sits at +0x14 of the object referenced from +8.
// FUNCTION: GP4 0x0045b4a0
void __fastcall FUN_0045b4a0(Unknown_0045b4a0* self, void* /*edx*/, int32_t value) {
    *(int32_t*)(self->Table08->Records14 + self->Index0c * 0x24) = value;
}
GP4_IMPL(0x0045b4a0, FUN_0045b4a0, "thiscall(ptr[0x10]:{8:ptr[0x20]:{0x14:ptr[0x100]:bytes}, 0xc:i32[0..7]}, i32) -> void")
