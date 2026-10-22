#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044cb00 { uint8_t Unknown00[0x1c]; uint32_t Words1c[6]; };
#pragma pack(pop)

// Copies the six dwords at +0x1c of the object into the buffer passed on the stack; returns 0.
// FUNCTION: GP4 0x0044cb00
int __fastcall FUN_0044cb00(Unknown_0044cb00* self, void* /*edx*/, uint32_t* dest) {
    for (int i = 0; i < 6; i++) {
        dest[i] = self->Words1c[i];
    }
    return 0;
}
GP4_IMPL(0x0044cb00, FUN_0044cb00, "thiscall(ptr[0x40]:bytes, ptr[0x18]:bytes) -> i32")
