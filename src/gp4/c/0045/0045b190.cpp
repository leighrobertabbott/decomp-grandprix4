#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Record_0045b190 {
    uint32_t Value00;
    uint32_t Value04;
    uint32_t Value08;
    uint8_t Unknown0c[0x88];
};
struct Table_0045b190 {
    uint8_t Unknown00[8];
    Record_0045b190* Records08;
};
struct Unknown_0045b190 {
    uint8_t Unknown00[8];
    Table_0045b190* Table08;
    int32_t Index0c;
};
#pragma pack(pop)

// Reads the first three dwords of the 0x94-byte record selected by the index at +0xc (record array reached
// through the table pointer at +8) into the three output slots.
// FUNCTION: GP4 0x0045b190
void __fastcall FUN_0045b190(Unknown_0045b190* self, void* /*edx*/, uint32_t* out0, uint32_t* out1, uint32_t* out2) {
    *out0 = self->Table08->Records08[self->Index0c].Value00;
    *out1 = self->Table08->Records08[self->Index0c].Value04;
    *out2 = self->Table08->Records08[self->Index0c].Value08;
}
GP4_IMPL(0x0045b190, FUN_0045b190, "thiscall(ptr[0x10]:{8:ptr[0x10]:{8:ptr[0x400]:bytes}, 0xc:i32[0..3]}, ptr[4]:bytes, ptr[4]:bytes, ptr[4]:bytes) -> void")
