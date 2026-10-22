#include <gp4/gp4.h>

struct Unknown_005b41c0;
struct Unknown_005b41c0_Channel;

// Slot 0x78 of the object: a thiscall predicate taking the channel id (stack argument), result in al.
typedef uint8_t (__fastcall *Unknown_005b41c0_Slot78)(Unknown_005b41c0* self, void* edx, uint32_t id);
// Slot 0x48 of a channel object: a stdcall method (COM style, the object itself is the only argument).
typedef void (__stdcall *Unknown_005b41c0_Slot48)(Unknown_005b41c0_Channel* self);

#pragma pack(push, 1)
struct Unknown_005b41c0_Channel { uint32_t* Vtable00; };
struct Unknown_005b41c0 {
    uint32_t* Vtable00;
    uint8_t Unknown04[0x258];
    Unknown_005b41c0_Channel** Channels25c;
};
#pragma pack(pop)

// Asks the object (virtual slot 0x78) whether the channel id is valid; when it answers 1 and a channel
// object is stored for the low 16 bits of the id, calls that object's virtual slot 0x48 (stdcall, the
// object as argument) and returns 0; otherwise returns 1.
// FUNCTION: GP4 0x005b41c0
int32_t __fastcall FUN_005b41c0(Unknown_005b41c0* self, void* /*edx*/, uint32_t id) {
    Unknown_005b41c0_Slot78 isValid = (Unknown_005b41c0_Slot78)self->Vtable00[0x78 / 4];
    if (isValid(self, 0, id) == 1) {
        Unknown_005b41c0_Channel* channel = self->Channels25c[id & 0xffff];
        if (channel != 0) {
            Unknown_005b41c0_Slot48 method = (Unknown_005b41c0_Slot48)channel->Vtable00[0x48 / 4];
            method(channel);
            return 0;
        }
    }
    return 1;
}
GP4_IMPL(0x005b41c0, FUN_005b41c0, "thiscall(ptr[0x270]:{0:ptr[0x80]:{0x78:u32{=0x005c2460}}, 0x25c:ptr[0x80]:{0*32/4:nptr[16]:com=IDirectSoundBuffer}}, u32{=0,1,2,3,4,0x10002}) -> i32")
