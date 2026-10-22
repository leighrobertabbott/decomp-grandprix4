#include <gp4/gp4.h>

struct Unknown_005a9d80 { uint8_t Unknown00[0x70]; uint8_t Byte70; };

// Getter: returns the byte at +0x70 of the object.
// FUNCTION: GP4 0x005a9d80
uint8_t __fastcall FUN_005a9d80(Unknown_005a9d80* self, void* /*edx*/) { return self->Byte70; }
GP4_IMPL(0x005a9d80, FUN_005a9d80, "thiscall(ptr[0x78]:bytes) -> u8")
