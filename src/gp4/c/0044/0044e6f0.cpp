#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044e6f0 {
    const void* Vptr00;
    uint32_t Unknown04;
    float Matrix08[16];  // 4x4 matrix reset to identity
};
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f04e8 at +0, clears +4 and sets the 16
// floats at +8..+0x47 to the 4x4 identity matrix. Returns this.
// FUNCTION: GP4 0x0044e6f0
Unknown_0044e6f0* __fastcall FUN_0044e6f0(Unknown_0044e6f0* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f04e8);
    self->Unknown04 = 0;
    for (int i = 0; i < 16; i++) {
        self->Matrix08[i] = 0.0f;
    }
    self->Matrix08[0] = 1.0f;
    self->Matrix08[5] = 1.0f;
    self->Matrix08[10] = 1.0f;
    self->Matrix08[15] = 1.0f;
    return self;
}
GP4_IMPL(0x0044e6f0, FUN_0044e6f0, "thiscall(ptr[0x50]:bytes) -> i32")
