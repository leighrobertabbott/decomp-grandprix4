#include <gp4/gp4.h>

// A polymorphic child object; only the vtable slot at +0x1c is used here.
struct Unknown_0047f3a0_Item {
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
struct Unknown_0047f3a0 { uint8_t Unknown00[0xc]; Unknown_0047f3a0_Item* Items0c[20]; uint8_t Unknown5c[0xa4]; };
#pragma pack(pop)

// Tail-calls virtual method +0x1c of the child object at index with the argument 1.
// FUNCTION: GP4 0x0047f3a0
int __fastcall FUN_0047f3a0(Unknown_0047f3a0* self, void* /*edx*/, int index) {
    return self->Items0c[index]->Slot1c(1);
}
GP4_IMPL(0x0047f3a0, FUN_0047f3a0, "thiscall(ptr[0x68]:layout=list_0047f3a0, i32[0..19]) -> u8")
