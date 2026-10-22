#include <gp4/gp4.h>

// A polymorphic child object; only the vtable slot at +0x1c is used here.
struct Unknown_0047f330_Item {
    virtual int Slot00() = 0;
    virtual int Slot04() = 0;
    virtual int Slot08() = 0;
    virtual int Slot0c() = 0;
    virtual int Slot10() = 0;
    virtual int Slot14() = 0;
    virtual int Slot18() = 0;
    virtual int Slot1c(int arg) = 0;
};

#pragma pack(push, 1)
struct Unknown_0047f330 {
    uint8_t Unknown00[0xc];
    Unknown_0047f330_Item* Items0c[20];
    int32_t Count5c;
    uint8_t Unknown60[8];
};
#pragma pack(pop)

// Menu list: clamps the selection index (passed by pointer) into 0..count-1, calls virtual method +0x1c
// with 0 on every child to deselect it, then tail-calls the same method with 1 on the child at position
// count-1-index.
// FUNCTION: GP4 0x0047f330
int __fastcall FUN_0047f330(Unknown_0047f330* self, void* /*edx*/, int32_t* index) {
    if (*index < 0) {
        *index = 0;
    }
    if (*index >= self->Count5c) {
        *index = self->Count5c - 1;
    }
    for (int i = 0; i < self->Count5c; i++) {
        self->Items0c[i]->Slot1c(0);
    }
    return self->Items0c[self->Count5c - *index - 1]->Slot1c(1);
}
GP4_IMPL(0x0047f330, FUN_0047f330, "thiscall(ptr[0x68]:layout=list_0047f3a0, ptr[8]:{0:i32[-3..24]}) -> u8")
