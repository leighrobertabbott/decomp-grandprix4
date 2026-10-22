#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4fd0 {
    uint8_t Unknown00[4];
    uint8_t Armed04;
    uint8_t Unknown05[0x13c - 5];
    uint32_t Value13c;
    uint32_t Value140;
    uint32_t Value144;
};
#pragma pack(pop)

// Calls the empty routine 0x005ae990; when the first argument and the record pointer are non-null and the
// record's flag byte +4 is clear, calls it again, stores the arguments at +0x140 (second), +0x144 (first)
// and +0x13c (third) and sets the flag. Always returns 1.
// FUNCTION: GP4 0x004d4fd0
int __stdcall FUN_004d4fd0(uint32_t first, uint32_t second, uint32_t third, Unknown_004d4fd0* record) {
    gp4::cdecl_<0x005ae990, void>();
    if (first != 0 && record != nullptr && record->Armed04 == 0) {
        gp4::cdecl_<0x005ae990, void>();
        record->Value140 = second;
        record->Value144 = first;
        record->Value13c = third;
        record->Armed04 = 1;
    }
    return 1;
}
GP4_IMPL(0x004d4fd0, FUN_004d4fd0, "stdcall(u32{0,0,1,0x1234}, u32, u32, nptr[0x150]:{4:u8{=0,0,1}}) -> i32")
