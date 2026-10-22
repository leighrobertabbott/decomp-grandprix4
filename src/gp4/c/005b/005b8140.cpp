#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b8140 {
    uint8_t  Unknown00[0x210];
    uint8_t* Records210;
};
#pragma pack(pop)

// Record table at +0x210 (stride 0x2c): stores base(record[a] - 0x18) + b * 0x10 into the
// dword at record[a] - 0x14 and returns it.
// FUNCTION: GP4 0x005b8140
int32_t __fastcall FUN_005b8140(Unknown_005b8140* self, void* /*edx*/, int32_t a, int32_t b)
{
    uint8_t* rec = self->Records210 + a * 0x2c;
    *(int32_t*)(rec - 0x14) = *(int32_t*)(rec - 0x18) + b * 0x10;
    return *(int32_t*)(self->Records210 + a * 0x2c - 0x14);
}
GP4_IMPL(0x005b8140, FUN_005b8140, "thiscall(ptr[0x218]:{0x210:ptr[0x300]+0x80:bytes}, i32[-2..5], i32) -> i32")
