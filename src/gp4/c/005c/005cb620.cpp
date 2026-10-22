#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb620 {
    uint8_t Unknown00[0x18];
    int32_t Count18;
    uint8_t Unknown1c[8];
    const int16_t* Text24;
};
#pragma pack(pop)

// Narrows the UTF-16 string at +0x24 into the destination byte buffer, copying at most
// Count18 + 1 characters and stopping at a zero character; a negative count copies nothing.
// The destination is always zero terminated.
// FUNCTION: GP4 0x005cb620
void __fastcall FUN_005cb620(Unknown_005cb620* self, void* /*edx*/, uint8_t* dst) {
    int32_t i = 0;
    const int16_t* src = self->Text24;
    if (self->Count18 >= 0) {
        do {
            if (*src == 0) {
                break;
            }
            int16_t ch = *src;
            src = src + 1;
            *dst = (uint8_t)ch;
            dst = dst + 1;
            i = i + 1;
        } while (i <= self->Count18);
    }
    *dst = 0;
}
GP4_IMPL(0x005cb620, FUN_005cb620, "thiscall(ptr[0x30]:{0x18:i32{=-1,0,1,3,8}, 0x24:ptr[0x40]:u16{0,0x41,0x142,0x7f,0xffff}}, ptr[0x100]:bytes) -> void")
