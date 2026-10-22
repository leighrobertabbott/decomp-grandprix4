#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Entry_0044d3b0 { uint32_t Value; uint32_t Unknown04; };
struct Unknown_0044d3b0 {
    uint8_t Unknown00[0x34];
    uint32_t Table34[0x100];
    int8_t Flags434[0x140];
    uint8_t Slot574[0x100];
    Entry_0044d3b0 Entries674[0x100];
};
#pragma pack(pop)

// Looks up the value for an index: when the signed flag byte +0x434[index] is not negative and the
// slot byte +0x574[index] is not 0xff, the result is the first dword of the 8-byte entry
// +0x674[slot]; otherwise it is the dword +0x34[index]. Stores it through the out pointer and
// returns 0.
// FUNCTION: GP4 0x0044d3b0
int32_t __fastcall FUN_0044d3b0(Unknown_0044d3b0* self, void* /*edx*/, int32_t index, uint32_t* out) {
    if (self->Flags434[index] >= 0) {
        uint8_t slot = self->Slot574[index];
        if (slot != 0xff) {
            *out = self->Entries674[slot].Value;
            return 0;
        }
    }
    *out = self->Table34[index];
    return 0;
}
GP4_IMPL(0x0044d3b0, FUN_0044d3b0, "thiscall(ptr[0xe80]:bytes, i32[0..255], ptr[8]:bytes) -> i32")
