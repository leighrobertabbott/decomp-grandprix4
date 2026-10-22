#include <gp4/gp4.h>

struct Delegate_004cb890;
typedef void(__thiscall* DelegateMethod_004cb890)(Delegate_004cb890*);

#pragma pack(push, 1)
struct Delegate_004cb890 { DelegateMethod_004cb890* Vtable00; };
struct Unknown_004cb890 { uint8_t Unknown00[0x17d4]; Delegate_004cb890* Delegate17d4; uint8_t Unknown17d8[0x28]; };
#pragma pack(pop)

// Forwards to the virtual method at vtable +0x10 of the delegate object stored at +0x17d4 (tail call);
// does nothing when the delegate is null.
// FUNCTION: GP4 0x004cb890
void __fastcall FUN_004cb890(Unknown_004cb890* self, void* /*edx*/) {
    Delegate_004cb890* d = self->Delegate17d4;
    if (d != nullptr) {
        d->Vtable00[4](d);
    }
}
GP4_IMPL(0x004cb890, FUN_004cb890, "thiscall(ptr[0x17d8]:layout=owner_004cb890) -> void")
