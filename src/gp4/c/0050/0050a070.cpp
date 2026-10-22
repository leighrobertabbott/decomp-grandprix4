#include <gp4/gp4.h>

struct Object_0050a070;
typedef void(__thiscall* VirtualSlot1)(Object_0050a070* self);

#pragma pack(push, 1)
struct Object_0050a070 { VirtualSlot1* Vtable00; };
#pragma pack(pop)

// If the audio data transfer object at 0x00b1d3dc (vtable 0x5f0d78, built by 0x509f00) is set, calls
// its virtual slot 1 (vtable +4); returns 1.
// FUNCTION: GP4 0x0050a070
uint8_t __cdecl FUN_0050a070() {
    Object_0050a070* obj = GP4_GLOBAL(Object_0050a070*, 0x00b1d3dc);
    if (obj != nullptr) {
        obj->Vtable00[1](obj);
    }
    return 1;
}
GP4_IMPL(0x0050a070, FUN_0050a070, "cdecl() -> u8; gfields={0x00b1d3dc:nptr[0x17d8]:layout=owner_004cb890}")
