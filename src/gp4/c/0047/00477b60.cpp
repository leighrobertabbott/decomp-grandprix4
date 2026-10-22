#include <gp4/gp4.h>

struct Unknown_00477b60;

#pragma pack(push, 1)
struct Unknown_00477b60 {
    void* Reset00;
    void* Reset04;
    void* Reset08;
};
#pragma pack(pop)

// Runs the entry-reset routine 0x00474d40 on the three objects referenced from +0, +4 and +8
// (the last one by a tail jump in the original).
// FUN_00474d40 resets entries 0..Last468 of the object it is given.
// FUNCTION: GP4 0x00477b60
void __fastcall FUN_00477b60(Unknown_00477b60* self, void* /*edx*/) {
    gp4::thiscall_<0x00474d40, void>(self->Reset00);
    gp4::thiscall_<0x00474d40, void>(self->Reset04);
    gp4::thiscall_<0x00474d40, void>(self->Reset08);
}
GP4_IMPL(0x00477b60, FUN_00477b60, "thiscall(ptr[0x10]:{0:ptr[0xa00]:u32{=0xffffffff,0,1,2,3,0x80808080}, 4:ptr[0xa00]:u32{=0xffffffff,0,1,2,3,0x80808080}, 8:ptr[0xa00]:u32{=0xffffffff,0,1,2,3,0x80808080}}) -> void")
