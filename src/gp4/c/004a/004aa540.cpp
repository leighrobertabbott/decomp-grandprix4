#include <gp4/gp4.h>
#include <gp4/x87.h>

// When the dword [0x00a663b8] is nonzero, stores (float)count * [0x005f04e4] (FILD of the zero-extended count,
// FMUL by the float constant, all in 80 bits, rounded to a float at each store) into the four consecutive
// floats 0x0064b2a4/a8/ac/b0 (one scale for each of the four channels).
// FUNCTION: GP4 0x004aa540
void __cdecl FUN_004aa540() {
    using namespace gp4::x87;
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00a663b8);
    if (count != 0u) {
        const float scaled = narrow_f32(ext_i64(static_cast<int64_t>(count)) * ext_f32(GP4_GLOBAL(float, 0x005f04e4)));
        GP4_GLOBAL(float, 0x0064b2a4) = scaled;
        GP4_GLOBAL(float, 0x0064b2a8) = scaled;
        GP4_GLOBAL(float, 0x0064b2ac) = scaled;
        GP4_GLOBAL(float, 0x0064b2b0) = scaled;
    }
}
GP4_IMPL(0x004aa540, FUN_004aa540, "cdecl() -> void; globals=fuzz:i32")
