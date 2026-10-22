#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00452c60_Cell {
    uint16_t* Slots00;   // array of 16-bit values
    uint16_t Count04;    // number of values stored in Slots00
};
#pragma pack(pop)

// Appends a 16-bit value to the slot list of grid cell (x, y): the grid at [0x9fb0a0] has
// [0x9fb0ac] cells per row, each cell a pointer to 16-bit slots plus a count.
// FUNCTION: GP4 0x00452c60
void __cdecl FUN_00452c60(int value, int x, int y) {
    int index = GP4_GLOBAL(int, 0x009fb0ac) * y + x;
    Unknown_00452c60_Cell* cells = GP4_GLOBAL(Unknown_00452c60_Cell*, 0x009fb0a0);
    uint16_t count = cells[index].Count04;
    cells[index].Slots00[count] = (uint16_t)value;
    GP4_GLOBAL(Unknown_00452c60_Cell*, 0x009fb0a0)[index].Count04++;
}
GP4_IMPL(0x00452c60, FUN_00452c60, "cdecl(i32, i32[0..1], i32[0..1]) -> void; globals=layout=cells_00452c60")
