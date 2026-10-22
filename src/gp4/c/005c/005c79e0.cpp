#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c79e0 {
    uint8_t Unknown00[4];
    uint8_t* Array04;
};
#pragma pack(pop)

// Destroys and frees the new[]-style array of 0x14-byte elements stored at +4 (its element
// count sits in the dword just before the array), then clears the pointer. A null pointer
// does nothing.
// FUNCTION: GP4 0x005c79e0
void __fastcall FUN_005c79e0(Unknown_005c79e0* self, void* /*edx*/) {
    uint8_t* array = self->Array04;
    if (array != 0) {
        gp4::stdcall_<0x005dbbb0, int>(array, 0x14, *(int32_t*)(array - 4), 0x005ae990);
        gp4::cdecl_<0x00446a60, int>(array - 4);
        self->Array04 = 0;
    }
}
GP4_IMPL(0x005c79e0, FUN_005c79e0, "thiscall(ptr[0x10]:{4:nptr[0x84]+4:{0:u32[0..3]}}) -> void")
