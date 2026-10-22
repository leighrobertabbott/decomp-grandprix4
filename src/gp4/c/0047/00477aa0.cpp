#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00477aa0 {
    uint8_t Unknown00[0x86];
    uint16_t Text86[0x80];
};
#pragma pack(pop)

// Widens a NUL-terminated byte string (terminator included) into the 0x80-entry word buffer at
// +0x86 of the object. A string that does not fit (more than 0x80 bytes with its terminator) only
// clears the first word.
// FUNCTION: GP4 0x00477aa0
void __fastcall FUN_00477aa0(Unknown_00477aa0* self, void* /*edx*/, const uint8_t* text) {
    int length = 0;
    while (text[length++] != 0) {
    }
    if (length > 0x80) {
        self->Text86[0] = 0;
        return;
    }
    for (int i = 0; i < length; i++) {
        self->Text86[i] = text[i];
    }
}
GP4_IMPL(0x00477aa0, FUN_00477aa0, "thiscall(ptr[0x190]:bytes, ptr[0x400]:bytes) -> void")
