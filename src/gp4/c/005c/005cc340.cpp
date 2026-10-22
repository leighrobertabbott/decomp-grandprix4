#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cc340 { int16_t Values00[0x10000]; };
#pragma pack(pop)

// Returns whether the signed word at index (low 16 bits of the argument) of the word table at the
// start of the object is positive: `and eax, 0xffff; cmp word [ecx + eax*2], 0; setg al`.
// FUNCTION: GP4 0x005cc340
uint8_t __fastcall FUN_005cc340(Unknown_005cc340* self, void* /*edx*/, uint32_t index) {
    return self->Values00[index & 0xffff] > 0;
}
GP4_IMPL(0x005cc340, FUN_005cc340, "thiscall(ptr[0x20008]:i16[-3..3], u32) -> u8")
