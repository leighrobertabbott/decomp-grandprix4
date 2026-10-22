#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_487530 {
    uint8_t Unknown00[0x4a07];
    uint8_t Flag4a07;
};
#pragma pack(pop)

// Copies the 16 dwords at src into the 0x40-byte field at +0x334 of record `index` (records are
// 0x2a0 bytes apart), clears the byte at +0x190 of that record and sets the flag at +0x4a07.
// FUNCTION: GP4 0x00487530
void __fastcall FUN_00487530(Unknown_487530* self, void* /*edx*/, int index, const uint32_t* src) {
    uint8_t* record = reinterpret_cast<uint8_t*>(self) + index * 0x2a0;
    uint32_t* dst = reinterpret_cast<uint32_t*>(record + 0x334);
    for (int count = 0x10; count != 0; --count) {
        *dst++ = *src++;
    }
    record[0x190] = 0;
    self->Flag4a07 = 1;
}
GP4_IMPL(0x00487530, FUN_00487530, "thiscall(ptr[0x4a10]:bytes, i32[0..3], ptr[0x40]:bytes) -> void")
