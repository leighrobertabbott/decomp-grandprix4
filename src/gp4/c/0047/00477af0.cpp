#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00477af0 {
    uint8_t Unknown00[0x86];
    uint16_t Text86[0x80];
};
#pragma pack(pop)

// Converts a NUL-terminated byte string (terminator included) through the CRT routine 0x005dd19b into
// the 0x80-entry word buffer at +0x86 of the object. A string that does not fit (more than 0x80
// bytes with its terminator) only clears the first word. Returns the buffer pointer (+0x86).
// FUNCTION: GP4 0x00477af0
uint16_t* __fastcall FUN_00477af0(Unknown_00477af0* self, void* /*edx*/, const uint8_t* text) {
    uint16_t* dest = self->Text86;
    int length = 0;
    while (text[length++] != 0) {
    }
    if (length > 0x80) {
        dest[0] = 0;
        return dest;
    }
    for (int i = 0; i < length; i++) {
        dest[i] = static_cast<uint16_t>(gp4::cdecl_<0x005dd19b, uint32_t>(static_cast<uint32_t>(text[i])));
    }
    return dest;
}
GP4_IMPL(0x00477af0, FUN_00477af0, "thiscall(ptr[0x190]:bytes, ptr[0x400]:bytes) -> u32")
