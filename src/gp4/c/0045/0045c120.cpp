#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045c120 {
    uint32_t Vtable00;
    uint32_t Unknown04;
    int32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    uint32_t Matrix14[16];  // 4x4 identity as float bit patterns
};
#pragma pack(pop)

// Constructor: stores the vtable pointer 0x005f05c8 at +0, clears +4 and sets +8 to -1, clears +0xc and
// +0x10 and initialises the 4x4 float matrix at +0x14 to the identity. Returns this.
// FUNCTION: GP4 0x0045c120
Unknown_0045c120* __fastcall FUN_0045c120(Unknown_0045c120* self, void* /*edx*/) {
    self->Vtable00 = 0x005f05c8;
    self->Unknown04 = 0;
    self->Unknown08 = 0xffffffffu;
    self->Unknown10 = 0;
    self->Unknown0c = 0;
    self->Matrix14[14] = 0;
    self->Matrix14[13] = 0;
    self->Matrix14[12] = 0;
    self->Matrix14[11] = 0;
    self->Matrix14[9] = 0;
    self->Matrix14[8] = 0;
    self->Matrix14[7] = 0;
    self->Matrix14[6] = 0;
    self->Matrix14[4] = 0;
    self->Matrix14[3] = 0;
    self->Matrix14[2] = 0;
    self->Matrix14[1] = 0;
    self->Matrix14[15] = 0x3f800000u;
    self->Matrix14[10] = 0x3f800000u;
    self->Matrix14[5] = 0x3f800000u;
    self->Matrix14[0] = 0x3f800000u;
    return self;
}
GP4_IMPL(0x0045c120, FUN_0045c120, "thiscall(ptr[0x60]:bytes) -> i32")
