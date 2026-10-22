#include <gp4/gp4.h>

struct Unknown_004cb8e0;
typedef void (__fastcall *VirtualFn)(Unknown_004cb8e0* self, void* edx);

#pragma pack(push, 1)
struct Unknown_004cb8e0 { const VirtualFn* Vtable00; };
struct Unknown_004cb8e0Owner { uint8_t Unknown00[0x17d4]; Unknown_004cb8e0* Child17d4; uint8_t Unknown17d8[0x28]; };
#pragma pack(pop)

// Forwards to virtual method 9 (vtable +0x24) of the object held at +0x17d4,
// if there is one.
// FUNCTION: GP4 0x004cb8e0
void __fastcall FUN_004cb8e0(Unknown_004cb8e0Owner* self, void* edx) {
    Unknown_004cb8e0* child = self->Child17d4;
    if (child != nullptr) {
        child->Vtable00[9](child, edx);
    }
}
GP4_IMPL(0x004cb8e0, FUN_004cb8e0, "thiscall(ptr[0x17d8]:layout=owner_004cb890) -> void")
