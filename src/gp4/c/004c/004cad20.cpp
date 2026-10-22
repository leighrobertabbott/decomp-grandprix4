#include <gp4/gp4.h>

struct Owned_004cad20;
typedef void(__thiscall* DeletingDtor_004cad20)(Owned_004cad20*, int);

#pragma pack(push, 1)
struct Owned_004cad20 { DeletingDtor_004cad20* Vtable00; };
struct Unknown_004cad20 {
    uint32_t Vptr00;
    Owned_004cad20* Owned04;
};
#pragma pack(pop)

// Destructor body of the "F1 Audio Data Receiver" class (vtable 0x005f0d14): stores the vtable
// pointer, deletes the owned object at +4 through its virtual deleting destructor (vtable slot 0,
// flag 1) when it is non-null, then runs the base destructor 0x004cab40. (The SEH frame the compiler
// builds around the call is not reproduced.)
// FUNCTION: GP4 0x004cad20
void __fastcall FUN_004cad20(Unknown_004cad20* self, void* /*edx*/) {
    self->Vptr00 = 0x005f0d14;
    Owned_004cad20* owned = self->Owned04;
    if (owned != nullptr) {
        owned->Vtable00[0](owned, 1);
    }
    gp4::thiscall_<0x004cab40, void>(self);
}
GP4_IMPL(0x004cad20, FUN_004cad20, "thiscall(ptr[0x10]:{4:nptr[0x20]:{0:u32{=0x005f0d14}}}) -> void")
