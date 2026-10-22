#include <gp4/gp4.h>

// Picks the jam resource for a track-side object name: if the name stored in the pointer global 0x00a69878
// contains "gridline" (CRT strstr 0x005db400, string at 0x0064b180) the buffer is overwritten with
// "maps\resources\jams\gridline.jip" (CRT sprintf 0x005db088, string at 0x0064b15c); otherwise if it
// contains "gridleft" (0x0064b150) it becomes "maps\resources\jams\gridleft.jip" (0x0064b12c). Returns 1
// when replaced, 0 when neither name matched.
// FUNCTION: GP4 0x004aa170
int __cdecl FUN_004aa170() {
    char* name = GP4_GLOBAL(char*, 0x00a69878);
    if (gp4::cdecl_<0x005db400, char*>(name, GP4_ARRAY(const char, 0x0064b180)) != 0) {
        gp4::cdecl_<0x005db088, int>(name, GP4_ARRAY(const char, 0x0064b15c));
        return 1;
    }
    if (gp4::cdecl_<0x005db400, char*>(name, GP4_ARRAY(const char, 0x0064b150)) != 0) {
        gp4::cdecl_<0x005db088, int>(name, GP4_ARRAY(const char, 0x0064b12c));
        return 1;
    }
    return 0;
}
GP4_IMPL(0x004aa170, FUN_004aa170, "cdecl() -> i32; gfields={0x00a69878:ptr[0x80]:{0:u32{=0x64697267,0x41414141}, 4:u32{=0x656e696c,0x7466656c,0x41414141}, 8:u32{=0}}}")
