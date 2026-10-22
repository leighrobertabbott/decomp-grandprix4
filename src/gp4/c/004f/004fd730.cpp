#include <gp4/gp4.h>

// Appends one character to the text-capture buffer 0x00aa2e50 (capture index 0x00669ed8, -1 = off).
// When the row key differs from the last one stored in 0x00aa2d48 and characters are already
// captured, a newline (0x0a) is inserted first; the row key is then remembered.
// FUNCTION: GP4 0x004fd730
void __stdcall FUN_004fd730(uint8_t ch, uint32_t /*unused*/, int rowKey) {
    int index = GP4_GLOBAL(int, 0x00669ed8);
    if (index != -1) {
        if (GP4_GLOBAL(int, 0x00aa2d48) != rowKey && index > 0) {
            GP4_ARRAY(uint8_t, 0x00aa2e50)[index] = 0x0a;
            index++;
        }
        GP4_GLOBAL(int, 0x00aa2d48) = rowKey;
        GP4_ARRAY(uint8_t, 0x00aa2e50)[index] = ch;
        GP4_GLOBAL(int, 0x00669ed8) = index + 1;
    }
}
GP4_IMPL(0x004fd730, FUN_004fd730, "stdcall(u8, u32, i32{0,1,2,7}) -> void; globals=fuzz:i32+layout=log_004fd730")
