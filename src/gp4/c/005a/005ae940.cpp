#include <gp4/gp4.h>
#include <gp4/dsound.h>

#pragma pack(push, 1)
struct Unknown_005ae940 {
    uint8_t Unknown00[0x58];
    uint8_t* Table58;
    uint8_t* Table5c;
    uint8_t Unknown60[0x25c - 0x60];
    IDirectSoundBuffer8** Buffers25c;
};
#pragma pack(pop)

// Clears the byte at index (arg & 0xffff) of the table at +0x58, then, when the sound buffer with that
// index (pointer table at +0x25c) exists, calls its Play (vtable +0x30) with no priority and the
// looping flag set when the byte of the table at +0x5c equals 1. Returns 1.
// FUNCTION: GP4 0x005ae940
int32_t __fastcall FUN_005ae940(Unknown_005ae940* self, void* /*edx*/, uint32_t index) {
    index = index & 0xffff;
    self->Table58[index] = 0;
    IDirectSoundBuffer8* buffer = self->Buffers25c[index];
    if (buffer != nullptr) {
        buffer->Play(0, 0, self->Table5c[index] == 1);
    }
    return 1;
}
GP4_IMPL(0x005ae940, FUN_005ae940, "thiscall(ptr[0x270]:{0x58:ptr[0x10]:bytes, 0x5c:ptr[0x10]:u8{0,1,2}, 0x25c:ptr[0x10]:{0:nptr[16]:com=IDirectSoundBuffer, 4:nptr[16]:com=IDirectSoundBuffer, 8:nptr[16]:com=IDirectSoundBuffer, 0xc:nptr[16]:com=IDirectSoundBuffer}}, u32{0,1,2,3,0x10001,0x20002,0xffff0003}) -> i32")
