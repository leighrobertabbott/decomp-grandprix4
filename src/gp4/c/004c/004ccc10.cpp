#include <gp4/gp4.h>

struct Child_004ccc10;
typedef void(__thiscall* ChildMethod_004ccc10)(Child_004ccc10*);

#pragma pack(push, 1)
struct Child_004ccc10 { ChildMethod_004ccc10* Vtable00; };
struct Unknown_004ccc10 {
    uint8_t Unknown00[0x70];
    Child_004ccc10* Child70;
    Child_004ccc10* Row74[4];
    Child_004ccc10* Child84;
    Child_004ccc10* Child88;
    uint8_t Flag8c;
};
#pragma pack(pop)

// Calls the virtual method at vtable +4 of the child objects: +0x70 and +0x84 when non-null; and
// when the byte at +0x8c is 1 also the four children at +0x74..+0x80 and then +0x88. Returns 0.
// FUNCTION: GP4 0x004ccc10
int __fastcall FUN_004ccc10(Unknown_004ccc10* self, void* /*edx*/) {
    if (self->Child70 != nullptr) {
        self->Child70->Vtable00[1](self->Child70);
    }
    if (self->Child84 != nullptr) {
        self->Child84->Vtable00[1](self->Child84);
    }
    if (self->Flag8c == 1) {
        for (int i = 0; i < 4; i++) {
            Child_004ccc10* child = self->Row74[i];
            if (child != nullptr) {
                child->Vtable00[1](child);
            }
        }
        if (self->Child88 != nullptr) {
            self->Child88->Vtable00[1](self->Child88);
        }
    }
    return 0;
}
GP4_IMPL(0x004ccc10, FUN_004ccc10, "thiscall(ptr[0x98]:{0x70:nptr[0x98]:{0:u32{=0x005f0e00}}, 0x74*4/4:nptr[0x98]:{0:u32{=0x005f0e00}}, 0x84:nptr[0x98]:{0:u32{=0x005f0e00}}, 0x88:nptr[0x98]:{0:u32{=0x005f0e00}}, 0x8c:u8{=0,1}}) -> i32")
