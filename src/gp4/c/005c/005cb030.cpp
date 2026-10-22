#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb030 { uint8_t Unknown00[0x20]; int32_t Value20; uint8_t Unknown24[0xdc]; };
#pragma pack(pop)

// Tests whether the dword at +0x20 is greater than the argument, signed (`cmp eax, ecx; setg al; ret 4`).
// Only al is written: the upper 24 bits of eax keep the upper bits of the loaded dword.
// FUNCTION: GP4 0x005cb030
int32_t __fastcall FUN_005cb030(const Unknown_005cb030* self, void* /*edx*/, int32_t value) {
    const int32_t field = self->Value20;
    return (int32_t)(((uint32_t)field & 0xffffff00u) | (uint32_t)(field > value));
}
GP4_IMPL(0x005cb030, FUN_005cb030, "thiscall(ptr[0x100]:bytes, i32) -> i32")
