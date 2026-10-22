#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d9f30_Rec {
    uint8_t Unknown00;
    uint8_t Unknown01;
    uint8_t Flags02;
    uint8_t Unknown03[0x9d];
};
struct Unknown_004d9f30 {
    uint8_t Unknown00[0x48];
    Unknown_004d9f30_Rec* Records48;
};
// 6-dword event record passed by value (the caller copies it with rep movsd).
struct Unknown_004d9f30_Event {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Code08;
    int32_t Value0C;
    int32_t Unknown10;
    int32_t Unknown14;
};
#pragma pack(pop)

// Event codes 2..7 set (value != 0) or clear (value == 0, also marking the
// record dirty at +0) bits 0..5 of byte +2 of record `index` (stride 0xa0).
// FUNCTION: GP4 0x004d9f30
void __fastcall FUN_004d9f30(Unknown_004d9f30* self, void* /*edx*/, int index,
                             Unknown_004d9f30_Event ev, int /*unused*/) {
    uint8_t bit;
    switch (ev.Code08) {
    case 2: bit = 0x01; break;
    case 3: bit = 0x02; break;
    case 4: bit = 0x04; break;
    case 5: bit = 0x08; break;
    case 6: bit = 0x10; break;
    case 7: bit = 0x20; break;
    default: return;
    }
    if (ev.Value0C != 0) {
        self->Records48[index].Flags02 |= bit;
    } else {
        self->Records48[index].Unknown00 = 1;
        self->Records48[index].Flags02 &= (uint8_t)~bit;
    }
}
GP4_IMPL(0x004d9f30, FUN_004d9f30, "thiscall(ptr[0x50]:{0x48:ptr[0x500]:bytes}, i32[0..7], i32, i32, i32[0..9], u32{0,1,0x100}, i32, i32, i32) -> void")
