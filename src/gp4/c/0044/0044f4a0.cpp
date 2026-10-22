#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044f4a0 {
    uint8_t Unknown00[0x457];
    char NameCopy457[0x80];
    char TextCopy4d7[0x10];
    char Names4e7[4][0x80];
    char Texts6e7[4][0x80];
    uint8_t Unknown8e7;
    int32_t Count8e8;
};
#pragma pack(pop)

// Pops the last entry of two parallel stacks of 0x80-byte strings (count at +0x8e8): the popped entries are
// copied, NUL-terminated, to the fixed destinations at +0x457 and +0x4d7.
// FUNCTION: GP4 0x0044f4a0
void __fastcall FUN_0044f4a0(Unknown_0044f4a0* self, void* /*edx*/) {
    int32_t index = self->Count8e8 - 1;
    self->Count8e8 = index;

    const char* src = self->Names4e7[0] + index * 0x80;
    char* dst = self->NameCopy457;
    char c;
    do {
        c = *src++;
        *dst++ = c;
    } while (c != '\0');

    src = self->Texts6e7[0] + self->Count8e8 * 0x80;
    dst = self->TextCopy4d7;
    do {
        c = *src++;
        *dst++ = c;
    } while (c != '\0');
}
GP4_IMPL(0x0044f4a0, FUN_0044f4a0, "thiscall(ptr[0x910]:{0x4e7*1024/1:u8{=0,0,0,0x41,0x42}, 0x8e8:u32[1..4]}) -> void")
