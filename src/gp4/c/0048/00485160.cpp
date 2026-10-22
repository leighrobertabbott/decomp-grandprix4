#include <gp4/gp4.h>

// Fills three out-parameters: *out_a = 0x33, *out_b = 2 and *out_c = entry 0x12 (when the byte flag
// at 0x00a4d5e8 is set) or entry 0x0d of the dword array read by FUN_0045ee70. The first argument
// is unused.
// FUNCTION: GP4 0x00485160
void __cdecl FUN_00485160(int /*unused*/, uint32_t* out_a, uint32_t* out_b, uint32_t* out_c) {
    *out_a = 0x33;
    *out_b = 2;
    if (GP4_GLOBAL(uint8_t, 0x00a4d5e8) != 0) {
        *out_c = gp4::cdecl_<0x0045ee70, uint32_t>(0x12);
        return;
    }
    *out_c = gp4::cdecl_<0x0045ee70, uint32_t>(0x0d);
}
GP4_IMPL(0x00485160, FUN_00485160, "cdecl(i32, ptr[8]:bytes, ptr[8]:bytes, ptr[8]:bytes) -> void; globals=fuzz:i32")
