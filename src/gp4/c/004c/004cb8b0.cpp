#include <gp4/gp4.h>

struct Delegate_004cb8b0;
typedef void(__thiscall* DelegateMethod_004cb8b0)(Delegate_004cb8b0*);

#pragma pack(push, 1)
struct Delegate_004cb8b0 { DelegateMethod_004cb8b0* Vtable00; };
struct Unknown_004cb8b0 { uint8_t Unknown00[0x17d4]; Delegate_004cb8b0* Delegate17d4; uint8_t Unknown17d8[0x28]; };
#pragma pack(pop)

// Forwards to the virtual method at vtable +0x14 of the delegate object stored at +0x17d4 (tail call);
// does nothing when the delegate is null.
// FUNCTION: GP4 0x004cb8b0
void __fastcall FUN_004cb8b0(Unknown_004cb8b0* self, void* /*edx*/) {
    Delegate_004cb8b0* d = self->Delegate17d4;
    if (d != nullptr) {
        d->Vtable00[5](d);
    }
}
GP4_IMPL(0x004cb8b0, FUN_004cb8b0, "thiscall(ptr[0x17d8]:layout=owner_004cb890) -> void")
