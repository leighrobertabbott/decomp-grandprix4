#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d1dd0 {
    uint8_t Unknown00[0xc];
    uint8_t* Array0c;
};
#pragma pack(pop)

// Destroys and frees the new[]-style array of 0x7c-byte elements stored at +0xc (its element count
// sits in the dword just before the array, the element destructor is 0x005d08f0), then clears the
// pointer. A null pointer only clears the field.
// FUNCTION: GP4 0x005d1dd0
void __fastcall FUN_005d1dd0(Unknown_005d1dd0* self, void* /*edx*/) {
    uint8_t* array = self->Array0c;
    if (array != 0) {
        gp4::stdcall_<0x005dbbb0, int>(array, 0x7c, *(int32_t*)(array - 4), 0x005d08f0);
        gp4::cdecl_<0x00446a60, int>(array - 4);
        self->Array0c = 0;
        return;
    }
    self->Array0c = 0;
}
GP4_IMPL(0x005d1dd0, FUN_005d1dd0, "thiscall(ptr[0x20]:{0xc:nptr[0x180]+4:{0:u32[0..3]}}) -> void")
