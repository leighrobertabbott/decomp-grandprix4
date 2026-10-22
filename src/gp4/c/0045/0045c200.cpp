#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_0045c200 {
    const void* Vptr00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0c;
    int32_t Unknown10;
    uint8_t Unknown14[0xc];
    uint32_t Unknown20;
    uint8_t Unknown24[2];
    uint8_t Unknown26;
};
#pragma pack(pop)

// Constructor: sets the vtable pointer 0x005f05cc (same class as the destructor 0x0045c230), zeroes the
// fields at +4, +8, +0xc, +0x20, +0x26, sets +0x10 to -1 and returns the object.
// FUNCTION: GP4 0x0045c200
Object_0045c200* __fastcall FUN_0045c200(Object_0045c200* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f05cc);
    self->Unknown04 = 0;
    self->Unknown08 = 0;
    self->Unknown0c = 0;
    self->Unknown10 = -1;
    self->Unknown20 = 0;
    self->Unknown26 = 0;
    return self;
}
GP4_IMPL(0x0045c200, FUN_0045c200, "thiscall(ptr[0x30]:bytes) -> i32")
