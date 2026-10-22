#include <gp4/gp4.h>

struct Unknown_005b3930_Buffer;

// Slot 0x4c of the 3D sound buffer (IDirectSound3DBuffer::SetPosition): stdcall, the object first.
typedef int32_t (__stdcall *Unknown_005b3930_Slot4c)(Unknown_005b3930_Buffer* self, uint32_t x, uint32_t y, uint32_t z, uint32_t apply);

#pragma pack(push, 1)
struct Unknown_005b3930_Buffer { uint32_t* Vtable00; };
struct Unknown_005b3930 {
    uint8_t Unknown00[0x260];
    Unknown_005b3930_Buffer** Buffers260;
};
#pragma pack(pop)

// Looks up the 3D buffer of the channel given by the low 16 bits of the id in the table at +0x260 and
// calls its virtual slot 0x4c with the three value arguments and 1; returns whether the call returned
// non-zero (0xbbf when the channel has no buffer).
// FUNCTION: GP4 0x005b3930
uint32_t __fastcall FUN_005b3930(Unknown_005b3930* self, void* /*edx*/, uint32_t id, uint32_t x, uint32_t y, uint32_t z) {
    Unknown_005b3930_Buffer* buffer = self->Buffers260[id & 0xffff];
    if (buffer == 0) {
        return 0xbbf;
    }
    Unknown_005b3930_Slot4c method = (Unknown_005b3930_Slot4c)buffer->Vtable00[0x4c / 4];
    return method(buffer, x, y, z, 1) != 0;
}
GP4_IMPL(0x005b3930, FUN_005b3930, "thiscall(ptr[0x270]:{0x260:ptr[0x80]:{0*32/4:nptr[16]:com=IDirectSound3DBuffer}}, u32{=0,1,2,3,0x10005}, i32, i32, i32) -> i32")
