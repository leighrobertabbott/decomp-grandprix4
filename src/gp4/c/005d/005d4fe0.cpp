#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005d4fe0 {
    uint8_t Unknown00[0x174]; float Value174;
    uint8_t Unknown178[0x44]; float Value1bc;
    uint8_t Unknown1c0[0x20];
};
#pragma pack(pop)

// Returns the float at +0x174 divided by the float at +0x1bc, left in st0
// (`fld dword [ecx+0x174]; fdiv dword [ecx+0x1bc]; ret`). The original leaves the extended-precision
// quotient in st0; a compiled C function can only return the double-rounded value, hence the
// tolerance (the same limitation as the sibling 0x005a7a30 sqrt).
// FUNCTION: GP4 0x005d4fe0
double __fastcall FUN_005d4fe0(Unknown_005d4fe0* self, void* /*edx*/) {
    using namespace gp4::x87;
    return narrow(ext_f32(self->Value174) / ext_f32(self->Value1bc));
}
GP4_IMPL(0x005d4fe0, FUN_005d4fe0, "thiscall(ptr[0x1e0]:f32) -> f64; tol=1e-12")
