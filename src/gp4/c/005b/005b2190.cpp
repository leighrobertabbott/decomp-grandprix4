#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b2190 {
    uint8_t Unknown00[0xc];
    int32_t* States0c;
    uint8_t* Flags10;
};
#pragma pack(pop)

// Index (a & 0xffff) of a state table at +0xc: if that state is 1 it is reset to 0, the matching
// flag byte of the array at +0x10 is set to 0xff and 0 is returned; any other state returns 0x7d1.
// FUNCTION: GP4 0x005b2190
int32_t __fastcall FUN_005b2190(Unknown_005b2190* self, void* /*edx*/, uint32_t index) {
    index = index & 0xffff;
    if (self->States0c[index] == 1) {
        self->States0c[index] = 0;
        self->Flags10[index] = 0xff;
        return 0;
    }
    return 0x7d1;
}
GP4_IMPL(0x005b2190, FUN_005b2190, "thiscall(ptr[0x14]:{0xc:ptr[0x40]:u32{0,1,2}, 0x10:ptr[0x10]:bytes}, u32{=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0x10003,0x2000a,0xffff0007}) -> i32")
