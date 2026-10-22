#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Entry_0044d3f0 { uint32_t Value; uint32_t Unknown04; };
struct Unknown_0044d3f0 {
    uint8_t Unknown00[0xe78];
    uint32_t Table0E78[0x100];
    int8_t Flags1278[0x100];
    int32_t Slot1378[0x100];
    Entry_0044d3f0 Entries1778[0x100];
};
#pragma pack(pop)

// Looks up the value for index = group * 32 + sub: when the signed flag byte +0x1278[index] is not
// negative and the slot dword +0x1378[index] is not -1, the result is the first dword of the 8-byte
// entry +0x1778[slot]; otherwise it is the dword +0xe78[index]. Stores it through the out pointer
// and returns 0.
// FUNCTION: GP4 0x0044d3f0
int32_t __fastcall FUN_0044d3f0(Unknown_0044d3f0* self, void* /*edx*/, int32_t group, int32_t sub, uint32_t* out) {
    int32_t index = group * 0x20 + sub;
    if (self->Flags1278[index] >= 0) {
        int32_t slot = self->Slot1378[index];
        if (slot != -1) {
            *out = self->Entries1778[slot].Value;
            return 0;
        }
    }
    *out = self->Table0E78[index];
    return 0;
}
GP4_IMPL(0x0044d3f0, FUN_0044d3f0, "thiscall(ptr[0x2000]:{0x1278*256/1:i8[-2..2], 0x1378*256/4:i32[-1..255]}, i32[0..7], i32[0..31], ptr[8]:bytes) -> i32")
