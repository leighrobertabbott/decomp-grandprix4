#include <gp4/gp4.h>

// Interface embedded at +0xc of the object held at +0x8c; only its first virtual method is used here.
struct Unknown_004d28d0_Sub {
    virtual void Slot0() = 0;
};

#pragma pack(push, 1)
struct Unknown_004d28d0_Held { uint8_t Unknown00[0xc]; uint8_t Sub0c[4]; };
struct Unknown_004d28d0 { uint8_t Unknown00[0x8c]; Unknown_004d28d0_Held* Held8c; };
#pragma pack(pop)

// Calls the empty routine 0x005ae990, then calls virtual method 0 of the interface embedded at +0xc of the
// object stored at +0x8c, and returns 0.
// Emulation fill: the held object's vptr is the vtable 0x005f0ee4 that the 0x004d2980 constructor stores at +0xc of
// its own object; the real class of the object at +0x8c is not confirmed.
// FUNCTION: GP4 0x004d28d0
int __fastcall FUN_004d28d0(Unknown_004d28d0* self, void* /*edx*/) {
    gp4::cdecl_<0x005ae990, void>();
    Unknown_004d28d0_Held* held = self->Held8c;
    ((Unknown_004d28d0_Sub*)held->Sub0c)->Slot0();
    return 0;
}
GP4_IMPL(0x004d28d0, FUN_004d28d0, "thiscall(ptr[0x90]:{0x8c:ptr[0x20]:{0xc:u32{=0x5f0ee4}}}) -> i32")
