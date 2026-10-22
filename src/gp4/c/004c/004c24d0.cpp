#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Record_004c24d0 { uint32_t Kind00; uint8_t Unknown04[0x94]; };
struct Unknown_004c24d0 {
    uint8_t Unknown00[0x7c];
    Record_004c24d0 Records7c[22];
    uint8_t Unknownd8c[0xe30 - 0xd8c];
    uint32_t Value0e30;
};
#pragma pack(pop)

// Resets the 22 records (0x98 bytes apart, starting at +0x7c) to kind 0x20000000 and clears
// the dword at +0xe30.
// FUNCTION: GP4 0x004c24d0
void __fastcall FUN_004c24d0(Unknown_004c24d0* self, void* /*edx*/) {
    for (int i = 0; i < 22; ++i) {
        self->Records7c[i].Kind00 = 0x20000000;
    }
    self->Value0e30 = 0;
}
GP4_IMPL(0x004c24d0, FUN_004c24d0, "thiscall(ptr[0xe40]:bytes) -> void")
