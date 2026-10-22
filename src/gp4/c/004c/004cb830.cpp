#include <gp4/gp4.h>

struct Object_004cb830;
struct Listener_004cb830;
typedef void (__fastcall *VirtualSlot1)(Listener_004cb830* self, void* edx, void* arg);

#pragma pack(push, 1)
struct Listener_004cb830 { const uint32_t* Vptr00; };
struct Object_004cb830 {
    uint8_t Unknown00[0xc];
    uint8_t Sub0c[0x17c8];
    Listener_004cb830* Listener17d4;
};
#pragma pack(pop)

// Forwards to the listener at +0x17d4 (when set): virtual slot 1 (vtable +4) with the address
// of the embedded sub-object at +0xc as its stack argument.
// FUNCTION: GP4 0x004cb830
void __fastcall FUN_004cb830(Object_004cb830* self, void* /*edx*/) {
    Listener_004cb830* listener = self->Listener17d4;
    if (listener != nullptr) {
        const uint32_t* vtable = listener->Vptr00;
        ((VirtualSlot1)vtable[1])(listener, (void*)vtable, self->Sub0c);
    }
}
GP4_IMPL(0x004cb830, FUN_004cb830, "thiscall(ptr[0x1800]:{0x17d4:nptr[0x40]:{0:u32{=0x005f0d14}, 4:ptr[0x280]:{0x234:u32{=0}}}}) -> void")
