#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044f5c0 { uint8_t Unknown00[0x14b]; char Text14b[0x104]; char Copy24f[0x104]; };
#pragma pack(pop)

// Copies the NUL-terminated string at +0x14b (terminator included) to +0x24f.
// FUNCTION: GP4 0x0044f5c0
void __fastcall FUN_0044f5c0(Unknown_0044f5c0* self, void* /*edx*/) {
    const char* src = self->Text14b;
    char* dst = self->Copy24f;
    char c;
    do {
        c = *src++;
        *dst++ = c;
    } while (c != '\0');
}
GP4_IMPL(0x0044f5c0, FUN_0044f5c0, "thiscall(ptr[0x400]:{0x14b*260/1:u8{=0,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48}}) -> void")
