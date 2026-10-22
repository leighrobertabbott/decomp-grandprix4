#include <gp4/gp4.h>

// Appends a message string to the 4-entry log buffer ending at 0x00829261: when the 16-byte id passed
// in is the one stored at 0x005f6300 and fewer than 4 entries are stored (count at 0x0082915a), copies
// up to 63 characters of the text into the buffer at the write pointer 0x0082915e (stopping at the end
// of the buffer or the terminator), terminates it, advances the write pointer past the terminator and
// bumps the count. Always returns 1.
// FUNCTION: GP4 0x005075f0
int __stdcall FUN_005075f0(const uint32_t* id, int /*unused*/, const char* text, int /*unused*/) {
    const uint32_t* wanted = GP4_ARRAY(const uint32_t, 0x005f6300);
    if (id[0] == wanted[0] && id[1] == wanted[1] && id[2] == wanted[2] && id[3] == wanted[3] &&
        GP4_GLOBAL(uint32_t, 0x0082915a) < 4) {
        char* dst = GP4_GLOBAL(char*, 0x0082915e);
        char* const end = GP4_ARRAY(char, 0x00829261);
        int remaining = 0x3f;
        while (dst != end) {
            char c = *text++;
            if (c == 0) break;
            *dst++ = c;
            if (--remaining == 0) break;
        }
        *dst++ = 0;
        GP4_GLOBAL(char*, 0x0082915e) = dst;
        GP4_GLOBAL(uint32_t, 0x0082915a)++;
    }
    return 1;
}
GP4_IMPL(0x005075f0, FUN_005075f0, "stdcall(u32{=0x5f6300,0x829162,0x829300}, i32, ptr[0x100]:{0*16/4:u32{=0x41414141,0x42424242,0x43434343,0x44444444,0x45454545,0x46464646,0x47474747,0x48484848,0x49494949,0x4a4a4a4a,0x4b4b4b4b,0x4c4c4c4c,0x4d4d4d4d,0x4e4e4e4e,0x4f4f4f4f,0x50505050,0x51515151,0x52525252,0x53535353,0x00424242}}, i32) -> i32; gfields={0x0082915a:u32[0..5], 0x0082915e:u32{=0x829162,0x829180,0x829200,0x829240,0x829261}}")
