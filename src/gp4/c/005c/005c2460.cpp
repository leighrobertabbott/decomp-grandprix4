#include <gp4/gp4.h>

// Stores the truth value of the byte argument into the first byte of the object and returns it in al
// (`mov al,[esp+4]; test al,al; setne al; mov [ecx],al; ret 4`).
// FUNCTION: GP4 0x005c2460
uint8_t __fastcall FUN_005c2460(uint8_t* self, void* /*edx*/, uint8_t value) {
    const uint8_t flag = (value != 0);
    *self = flag;
    return flag;
}
GP4_IMPL(0x005c2460, FUN_005c2460, "thiscall(ptr[0x100]:bytes, i32) -> u8")
