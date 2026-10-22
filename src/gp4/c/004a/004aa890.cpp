#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Rec_004aa890 {
    uint32_t Value00;
    uint32_t Value04;
    uint32_t Value08;
    uint8_t Unknown0c[0x10];
};
struct Mark_004aa890 {
    int32_t Value00;
    uint8_t Unknown04[0x74];
};
struct Unknown_004aa890 {
    uint32_t Vtable00;
    uint32_t Value04;
    Rec_004aa890 Recs08[0x226];
    uint32_t Value3c30;
    uint8_t Unknown3c34[8];
    uint32_t Value3c3c;
    uint8_t Unknown3c40[0x3ca4 - 0x3c40];
    Mark_004aa890 Marks3ca4[0x16];
};
#pragma pack(pop)

// Constructor: stores the argument at +4 and the vtable pointer 0x005f0b04 at +0, clears the dwords at
// +0x3c30 and +0x3c3c, clears the first three dwords of each of the 0x226 28-byte records at +8, sets the
// first dword of each of the 0x16 120-byte entries at +0x3ca4 to -1 and clears the dword at +0x46a4.
// Returns this (eax = ecx), which the caller at 0x496818 stores.
// FUNCTION: GP4 0x004aa890
Unknown_004aa890* __fastcall FUN_004aa890(Unknown_004aa890* self, void* /*edx*/, uint32_t value) {
    self->Value04 = value;
    self->Vtable00 = 0x005f0b04;
    self->Value3c30 = 0;
    self->Value3c3c = 0;
    for (int i = 0; i < 0x226; i++) {
        self->Recs08[i].Value00 = 0;
        self->Recs08[i].Value04 = 0;
        self->Recs08[i].Value08 = 0;
    }
    for (int i = 0; i < 0x16; i++) {
        self->Marks3ca4[i].Value00 = -1;
    }
    GP4_FIELD(uint32_t, self, 0x46a4) = 0;
    return self;
}
GP4_IMPL(0x004aa890, FUN_004aa890, "thiscall(ptr[0x4700]:bytes, u32) -> u32")
