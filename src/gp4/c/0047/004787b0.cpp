#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004787b0 {
    uint8_t Unknown00[0x54];
    int32_t Unknown54;
    uint8_t Unknown58[0xc];
    int32_t Unknown64;
    uint8_t Unknown68[4];
    int32_t Unknown6C;
    uint8_t Unknown70[8];
    int32_t Unknown78;
    int32_t Unknown7C;
    int32_t Unknown80;
    int32_t Unknown84;
    int32_t Unknown88;
    int32_t Unknown8C;
    int32_t Unknown90;
    int32_t Unknown94;
    int32_t Unknown98;
    int32_t Unknown9C;
    int32_t UnknownA0;
    int32_t UnknownA4;
    int32_t UnknownA8;
    int32_t UnknownAC;
    int32_t UnknownB0;
    int32_t UnknownB4;
    int32_t UnknownB8;
    int32_t UnknownBC;
    int32_t UnknownC0;
    int32_t UnknownC4;
    int32_t UnknownC8;
    int32_t UnknownCC;
    int32_t UnknownD0;
    int32_t UnknownD4;
    int32_t UnknownD8;
    int32_t UnknownDC;
    int32_t UnknownE0;
    int32_t UnknownE4;
};
#pragma pack(pop)

// Clears the dword switches +0x54, +0x64, +0x6c and +0x78..+0xe4 of obj, then turns on a set
// that grows with level (0: none, 1, 2, any other value: all cumulative sets; the byte flag adds
// +0x54/+0xe4 and +0x64 = 1, or +0x64 = 2 and +0x78 at the top level before +0x64 is set to 1).
// FUNCTION: GP4 0x004787b0
void __cdecl FUN_004787b0(Unknown_004787b0* obj, int level, char flag) {
    obj->Unknown54 = 0;
    obj->Unknown64 = 0;
    obj->Unknown6C = 0;
    obj->Unknown78 = 0;
    obj->Unknown7C = 0;
    obj->Unknown80 = 0;
    obj->Unknown84 = 0;
    obj->Unknown88 = 0;
    obj->Unknown8C = 0;
    obj->Unknown90 = 0;
    obj->Unknown94 = 0;
    obj->Unknown98 = 0;
    obj->Unknown9C = 0;
    obj->UnknownA0 = 0;
    obj->UnknownA4 = 0;
    obj->UnknownA8 = 0;
    obj->UnknownAC = 0;
    obj->UnknownB0 = 0;
    obj->UnknownB4 = 0;
    obj->UnknownB8 = 0;
    obj->UnknownBC = 0;
    obj->UnknownC0 = 0;
    obj->UnknownC4 = 0;
    obj->UnknownC8 = 0;
    obj->UnknownCC = 0;
    obj->UnknownD0 = 0;
    obj->UnknownD4 = 0;
    obj->UnknownD8 = 0;
    obj->UnknownDC = 0;
    obj->UnknownE0 = 0;
    obj->UnknownE4 = 0;

    switch (level) {
    case 0:
        return;
    default:
        if (flag != 0) {
            obj->Unknown64 = 2;
            obj->Unknown78 = 1;
        }
        obj->Unknown88 = 1;
        obj->UnknownBC = 1;
        obj->UnknownC4 = 1;
        obj->UnknownD8 = 1;
        // fall through
    case 2:
        obj->Unknown54 = 1;
        obj->UnknownE4 = 1;
        if (flag != 0) {
            obj->Unknown64 = 1;
        }
        obj->Unknown6C = 1;
        obj->Unknown7C = 1;
        obj->Unknown80 = 1;
        obj->Unknown84 = 1;
        obj->Unknown8C = 1;
        obj->Unknown94 = 1;
        obj->Unknown9C = 1;
        obj->UnknownA0 = 1;
        obj->UnknownA4 = 1;
        obj->UnknownB8 = 1;
        obj->UnknownD4 = 1;
        obj->UnknownDC = 1;
        obj->UnknownE0 = 1;
        // fall through
    case 1:
        if (flag != 0) {
            obj->Unknown54 = 1;
            obj->UnknownE4 = 1;
        }
        obj->UnknownCC = 1;
        obj->UnknownD0 = 1;
        obj->UnknownB4 = 1;
        obj->UnknownC0 = 1;
        obj->UnknownC8 = 1;
        obj->Unknown90 = 1;
        obj->Unknown98 = 1;
        obj->UnknownA8 = 1;
        obj->UnknownAC = 1;
        obj->UnknownB0 = 1;
        break;
    }
}
GP4_IMPL(0x004787b0, FUN_004787b0, "cdecl(ptr[0xf0]:bytes, i32{0,1,2,3,-1}, u32{0,1,0x100,0xffffff00}) -> void")
