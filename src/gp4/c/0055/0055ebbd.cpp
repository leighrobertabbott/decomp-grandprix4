#include <gp4/gp4.h>
#include <gp4/x87.h>

// Builds a 16-byte value in a stack temporary from three floats (each copied through the x87,
// fld dword / fstp dword, so a signalling NaN is quieted), then copies the temporary into `out`
// and returns `out`. Both helpers are tail-jump thunks into a statically linked class library.
// FUNCTION: GP4 0x0055ebbd
int __stdcall FUN_0055ebbd(void* out, float x, float y, float z) {
    using namespace gp4::x87;
    uint8_t temp[16];
    const float zc = narrow_f32(ext_f32(z));
    const float yc = narrow_f32(ext_f32(y));
    const float xc = narrow_f32(ext_f32(x));
    gp4::stdcall_<0x0055db25, void*>(static_cast<void*>(temp), xc, yc, zc);
    gp4::stdcall_<0x0055d0b3, void*>(out, static_cast<void*>(temp));
    return static_cast<int>(reinterpret_cast<uintptr_t>(out));
}
GP4_IMPL(0x0055ebbd, FUN_0055ebbd, "stdcall(ptr[0x10]:f32, f32, f32, f32) -> i32")
