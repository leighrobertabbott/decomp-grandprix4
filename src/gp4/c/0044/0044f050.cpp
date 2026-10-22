#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Entry_0044f050 {
    uint32_t Unknown00;
    int32_t Key04;
    uint32_t Unknown08;
};
struct Unknown_0044f050 {
    uint8_t Unknown00[0xba0];
    uint32_t Count0ba0;
    uint32_t Unknown0ba4;
    Entry_0044f050* Entries0ba8;
    uint32_t Index0bac;
};
#pragma pack(pop)

// Looks the key up in the table of 12-byte entries (count at +0xba0, array at +0xba8): the first entry
// whose dword +4 equals the key gets that dword cleared and its index stored at +0xbac. A null key is
// ignored.
// FUNCTION: GP4 0x0044f050
void __fastcall FUN_0044f050(Unknown_0044f050* self, void* /*edx*/, int32_t key) {
    if (key != 0) {
        uint32_t count = self->Count0ba0;
        if (count > 0) {
            Entry_0044f050* entries = self->Entries0ba8;
            uint32_t i = 0;
            do {
                if (entries[i].Key04 == key) {
                    entries[i].Key04 = 0;
                    self->Index0bac = i;
                    return;
                }
                i++;
            } while (i < count);
        }
    }
}
GP4_IMPL(0x0044f050, FUN_0044f050, "thiscall(ptr[0xbc0]:{0xba0:u32[0..8], 0xba8:ptr[0x80]:{4*8/12:u32[1..3]}}, i32[0..4]) -> void")
