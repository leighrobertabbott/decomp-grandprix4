#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00452c30_Cell {
    uint16_t* Slots00;   // array of 16-bit values
    uint16_t Count04;    // number of values stored in Slots00
};
#pragma pack(pop)

// Increments the slot count of grid cell (x, y) - the grid at [0x9fb0a0] has [0x9fb0ac] cells per
// row - and returns the address of the count. The first argument is unused.
// FUNCTION: GP4 0x00452c30
uint16_t* __cdecl FUN_00452c30(int /*unused*/, int x, int y) {
    int index = GP4_GLOBAL(int, 0x009fb0ac) * y + x;
    Unknown_00452c30_Cell* cells = GP4_GLOBAL(Unknown_00452c30_Cell*, 0x009fb0a0);
    cells[index].Count04++;
    return &cells[index].Count04;
}
GP4_IMPL(0x00452c30, FUN_00452c30, "cdecl(i32, i32[0..1], i32[0..1]) -> i32; globals=layout=cells_00452c60")
