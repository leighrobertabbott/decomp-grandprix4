#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c3b60 { uint8_t Unknown00[8]; uint8_t Flag08; uint8_t Unknown09[11]; uint8_t Flag14; };
#pragma pack(pop)

// Sets the byte flags at +0x08 and +0x14 to 1; leaves al = 1 (`mov al, 1; mov [ecx+8], al; mov [ecx+0x14], al`).
// FUNCTION: GP4 0x005c3b60
uint8_t __fastcall FUN_005c3b60(Unknown_005c3b60* self, void* /*edx*/) {
    self->Flag08 = 1;
    self->Flag14 = 1;
    return 1;
}
GP4_IMPL(0x005c3b60, FUN_005c3b60, "thiscall(ptr[0x100]:bytes) -> u8")
