#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00492dc0 {
    uint8_t Unknown00[0x4ac];
    int32_t Mode4ac;
};
struct Vec_00492dc0 {
    float X;
    uint32_t Y;
    uint32_t Z;
};
#pragma pack(pop)

// When the mode dword at +0x4ac is 3, copies two triples out of the stack arguments (the first
// value of each travels through the x87 stack as a float) into the two output vectors; otherwise
// clears both output vectors.
// FUNCTION: GP4 0x00492dc0
void __fastcall FUN_00492dc0(Unknown_00492dc0* self, void* /*edx*/, uint32_t a0, uint32_t a1, uint32_t a2,
                             uint32_t a3, float x1, uint32_t y1, uint32_t z1, uint32_t a7, float x2,
                             uint32_t y2, uint32_t z2, uint32_t a11, uint32_t a12, uint32_t a13,
                             uint32_t a14, uint32_t a15, Vec_00492dc0* out1, Vec_00492dc0* out2) {
    if (self->Mode4ac == 3) {
        out1->X = x1;
        out1->Y = y1;
        out1->Z = z1;
        out2->X = x2;
        out2->Z = z2;
        out2->Y = y2;
        return;
    }
    out1->Z = 0;
    out1->Y = 0;
    out1->X = 0;
    out2->Z = 0;
    out2->X = 0;
    out2->Y = 0;
}
GP4_IMPL(0x00492dc0, FUN_00492dc0, "thiscall(ptr[0x4c0]:{0x4ac:u32{=0,3,3,1}}, u32, u32, u32, u32, u32{0x3f800000,0x7fa00000,0x40490fdb,0xc0000000,0x00000001}, u32, u32, u32, u32{0x3f800000,0x7fa00000,0x40490fdb,0xc0000000,0xff800001}, u32, u32, u32, u32, u32, u32, u32, ptr[0x10]:bytes, ptr[0x10]:bytes) -> void")
