#include <gp4/gp4.h>
#include <gp4/dinput8.h>

#pragma pack(push, 1)
struct Unknown_004ad3d0 { IDirectInputDevice8A* Device00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Unacquires the DirectInput device held in the dword at +0, if any; returns its HRESULT (0 when null).
// FUNCTION: GP4 0x004ad3d0
int32_t __fastcall FUN_004ad3d0(Unknown_004ad3d0* self, void* /*edx*/) {
    IDirectInputDevice8A* dev = self->Device00;
    if (dev != nullptr) {
        return dev->Unacquire();
    }
    return 0;
}
GP4_IMPL(0x004ad3d0, FUN_004ad3d0, "thiscall(ptr[0x10]:layout=didev_004ad3d0) -> i32")
