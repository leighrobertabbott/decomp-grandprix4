#include <gp4/gp4.h>

// Seek wrapper: after the empty routine 0x005ae990 it maps the mode byte 0/1/2 to the C runtime's
// SEEK_SET/SEEK_CUR/SEEK_END origin and tail-calls the CRT fseek (0x005dcb47). Any other mode
// returns 1 without seeking.
// FUNCTION: GP4 0x004d4720
int __cdecl FUN_004d4720(void* file, int32_t offset, uint32_t mode) {
    gp4::cdecl_<0x005ae990, void>();
    int32_t origin;
    switch (mode & 0xffu) {
    case 0:
        origin = 0;
        break;
    case 1:
        origin = 1;
        break;
    case 2:
        origin = 2;
        break;
    default:
        return 1;
    }
    return gp4::cdecl_<0x005dcb47, int>(file, offset, origin);
}
GP4_IMPL(0x004d4720, FUN_004d4720, "cdecl(ptr[0x40]:bytes, i32, u32{0,1,2,3,0x101,0x102}) -> i32")
