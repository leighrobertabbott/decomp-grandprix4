#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Int16Pair { int16_t X; int16_t Y; };
#pragma pack(pop)

// Calls the car-slot selector 0x00510aee (which sets the index dword 0x00a69878), then stores the
// pair of 16-bit values at that index of the table 0x00644b50, offset by (+0x7a, -0xa0), in the two
// words 0x00679d06 / 0x00679d08.
// FUNCTION: GP4 0x0048caf0
void __cdecl FUN_0048caf0() {
    gp4::Regs c = {};
    gp4::call_regs(0x00510aee, c);
    const Int16Pair* table = GP4_ARRAY(const Int16Pair, 0x00644b50);
    uint32_t index = GP4_GLOBAL(uint32_t, 0x00a69878);
    int16_t x = static_cast<int16_t>(table[index].X + 0x7a);
    int16_t y = static_cast<int16_t>(table[index].Y - 0xa0);
    GP4_GLOBAL(int16_t, 0x00679d06) = x;
    GP4_GLOBAL(int16_t, 0x00679d08) = y;
}
GP4_IMPL(0x0048caf0, FUN_0048caf0, "cdecl() -> void; globals=fuzz:i32+layout=ptr_00679d30")
