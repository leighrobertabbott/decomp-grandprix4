#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044f450 {
    uint8_t Unknown00[0x457];
    char NameCopy457[0x80];
    char TextCopy4d7[0x10];
    char Names4e7[4][0x80];
    char Texts6e7[4][0x80];
    uint8_t Unknown8e7;
    int32_t Count8e8;
};
#pragma pack(pop)

// Pushes the strings at +0x457 and +0x4d7 onto two parallel stacks of 0x80-byte NUL-terminated strings
// (at +0x4e7 and +0x6e7, count at +0x8e8) and increments the count.
// FUNCTION: GP4 0x0044f450
void __fastcall FUN_0044f450(Unknown_0044f450* self, void* /*edx*/) {
    const char* src = self->NameCopy457;
    char* dst = self->Names4e7[0] + self->Count8e8 * 0x80;
    char c;
    do {
        c = *src++;
        *dst++ = c;
    } while (c != '\0');

    src = self->TextCopy4d7;
    dst = self->Texts6e7[0] + self->Count8e8 * 0x80;
    do {
        c = *src++;
        *dst++ = c;
    } while (c != '\0');

    self->Count8e8 = self->Count8e8 + 1;
}
GP4_IMPL(0x0044f450, FUN_0044f450, "thiscall(ptr[0x910]:{*bytes, 0x457*128/1:u8{=0x41,0x42,0}, 0x4d7*16/1:u8{=0x43,0}, 0x8e8:u32[0..3]}) -> void")
