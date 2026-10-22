#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004595b0_Mgr {
    uint8_t Unknown00[4];
    void* TexMgr04;
};
struct Unknown_004595b0 {
    uint8_t Unknown00[0x1ac];
    Unknown_004595b0_Mgr* Mgr1ac;
    uint8_t Unknown1b0[0x204 - 0x1b0];
    int32_t* Table204;
};
#pragma pack(pop)

// Stores `value` into slot `index` of the dword table at +0x204; when `release` is nonzero the
// previous entry is first released through 0x00451370 (called on [[this+0x1ac]+4]).
// FUNCTION: GP4 0x004595b0
void __fastcall FUN_004595b0(Unknown_004595b0* self, void* /*edx*/, int index, int32_t value, char release) {
    if (release != 0) {
        gp4::thiscall_<0x00451370, void>(self->Mgr1ac->TexMgr04, self->Table204[index]);
        self->Table204[index] = value;
        return;
    }
    self->Table204[index] = value;
}
GP4_IMPL(0x004595b0, FUN_004595b0, "thiscall(ptr[0x210]:{0x1ac:ptr[8]:{4:ptr[0x40]:layout=texmgr_00451540}, 0x204:ptr[0x20]:i32[-1..7]}, i32[0..7], i32, u8) -> void")
