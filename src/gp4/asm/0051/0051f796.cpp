#include <gp4/gp4.h>
#include <gp4/flags.h>

// Row offset: the original multiplies the row by the pitch with `mul`; only the low dword is used.
static __declspec(noinline) uint32_t rowOffset(uint32_t row, uint32_t pitch) {
    return row * pitch;
}

// Clears the 640x480 byte surface at [0x874ef0] (0x12c00 dwords) and draws a 0x33 pattern into it: nine 640-byte
// bands starting at row 0x70 (every 0x20 rows, the rows being [0x8751c4] bytes apart), then eleven 480-row vertical
// byte lines at x = 0xa0 + 0x20*k. pushal/popal keep every register; the flags left are those of the last
// `dec esi` (esi 1 -> 0) with the CF of the last `add edi, 0x20`.
// FUNCTION: GP4 0x0051f796
void __cdecl FUN_0051f796(gp4::Regs* r) {
    uint32_t* clear = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00874ef0)));
    for (uint32_t i = 0; i != 0x12c00u; ++i) {
        clear[i] = 0u;
    }

    for (int32_t row = 0x70; row <= 0x170; row += 0x20) {
        uint32_t* band = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(
            rowOffset(static_cast<uint32_t>(row), GP4_GLOBAL(uint32_t, 0x008751c4)) + GP4_GLOBAL(uint32_t, 0x00874ef0)));
        for (uint32_t i = 0; i != 0xa0u; ++i) {
            band[i] = 0x33333333u;
        }
    }

    uint32_t column = GP4_GLOBAL(uint32_t, 0x00874ef0) + 0xa0u;
    uint32_t addFlags = 0u;
    uint32_t columnsLeft = 0xbu;
    do {
        uint32_t p = column;
        uint32_t rows = 0x1e0u;
        do {
            *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(p)) = 0x33u;
            p += GP4_GLOBAL(uint32_t, 0x008751c4);
            rows = rows - 1u;
        } while (rows != 0u);
        addFlags = gp4::flags::add32(column, 0x20u);
        column += 0x20u;
        columnsLeft = columnsLeft - 1u;
    } while (columnsLeft != 0u);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, addFlags));
}
GP4_IMPL(0x0051f796, FUN_0051f796, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=surf_0051f796")
