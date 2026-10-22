#include <gp4/gp4.h>

struct Object_0046bbe0;
typedef void (__fastcall *VirtualSlot3)(Object_0046bbe0* self, void* edx);

#pragma pack(push, 1)
struct Object_0046bbe0 { const uint32_t* Vptr00; uint8_t Unknown04[0x28]; int32_t Value2c; };
#pragma pack(pop)

// Stores the argument at +0x2c, then calls virtual slot 3 (vtable +0xc) of the object.
// FUNCTION: GP4 0x0046bbe0
void __fastcall FUN_0046bbe0(Object_0046bbe0* self, void* /*edx*/, int32_t value) {
    const uint32_t* vtable = self->Vptr00;
    self->Value2c = value;
    ((VirtualSlot3)vtable[3])(self, (void*)vtable);
}
GP4_IMPL(0x0046bbe0, FUN_0046bbe0, "thiscall(ptr[0x40]:u32{=0x5f0758}, i32) -> void")
