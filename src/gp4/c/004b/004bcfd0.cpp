#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004bcfd0 { uint8_t Unknown00[0x1eec]; uint8_t Flag1eec; };
#pragma pack(pop)

// Sets the byte flag at +0x1eec of the object to 1.
// FUNCTION: GP4 0x004bcfd0
void __fastcall FUN_004bcfd0(Unknown_004bcfd0* self, void* /*edx*/) { self->Flag1eec = 1; }
GP4_IMPL(0x004bcfd0, FUN_004bcfd0, "thiscall(ptr[0x1f00]:bytes) -> void")
