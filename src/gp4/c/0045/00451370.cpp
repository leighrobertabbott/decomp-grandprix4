#include <gp4/gp4.h>

#pragma pack(push, 1)
// Reference record of the texture manager: the reference count dword first, 0x54 bytes in all.
struct TextureRef {
    uint32_t RefCount;
    uint8_t Unknown04[0x50];
};
struct Unknown_00451370 {
    uint8_t Unknown00[0x28];
    TextureRef* Refs28;
    uint8_t Unknown2c[8];
    int32_t FreeIndex34;
};
#pragma pack(pop)

// Drops one reference of record `index` (when it is not -1): the count at the start of the 0x54-byte
// record is decremented if nonzero, and when it is zero afterwards the index is stored at +0x34.
// FUNCTION: GP4 0x00451370
void __fastcall FUN_00451370(Unknown_00451370* self, void* /*edx*/, int index) {
    if (index != -1) {
        uint32_t count = self->Refs28[index].RefCount;
        if (count != 0) {
            self->Refs28[index].RefCount = count - 1;
        }
        if (self->Refs28[index].RefCount == 0) {
            self->FreeIndex34 = index;
        }
    }
}
GP4_IMPL(0x00451370, FUN_00451370, "thiscall(ptr[0x40]:layout=texmgr_00451540, i32[-1..7]) -> void")
