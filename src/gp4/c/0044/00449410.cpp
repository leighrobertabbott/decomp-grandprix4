#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00449410_Rec {
    uint8_t Unknown00[0x20];
    uint8_t Flag20;
    uint8_t Unknown21[0x4c - 0x21];
};
struct Unknown_00449410_TexMgr {
    uint8_t Unknown00[0x24];
    Unknown_00449410_Rec* Recs24;
};
struct Unknown_00449410_A {
    uint8_t Unknown00[4];
    Unknown_00449410_TexMgr* TexMgr04;
};
struct Unknown_00449410_Owner {
    uint8_t Unknown00[0x20c];
    uint32_t* Table20c;
};
struct Unknown_00449410_B {
    uint8_t Unknown00[4];
    Unknown_00449410_Owner* Owner04;
};
#pragma pack(pop)

// Looks up a texture index in the owner's table (-1 = none) and returns whether the flag byte
// at +0x20 of that 0x4c-byte texture record is nonzero.
// FUNCTION: GP4 0x00449410
bool __stdcall FUN_00449410(Unknown_00449410_A* a, Unknown_00449410_B* b, const int32_t* c) {
    uint32_t t = b->Owner04->Table20c[*c];
    if (t != 0xffffffffu) {
        if (a->TexMgr04->Recs24[t].Flag20 != 0) {
            return true;
        }
    }
    return false;
}
GP4_IMPL(0x00449410, FUN_00449410, "stdcall(ptr[0x10]:layout=rend_0044d280, ptr[8]:layout=mrec_00449410, ptr[4]:i32[0..7]) -> u8")
