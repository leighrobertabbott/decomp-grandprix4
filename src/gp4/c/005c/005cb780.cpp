#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb780 {
    uint8_t Unknown00[0x1c];
    int32_t Count1c;
    int32_t Position20;
    uint16_t* Text24;
};
#pragma pack(pop)

// Deletes the 16-bit character before the cursor at +0x20 from the zero-terminated text at +0x24
// (the tail, terminator included, is shifted down by one), then decrements the length at +0x1c.
// Does nothing when the cursor is 0 or the length is not positive.
// FUNCTION: GP4 0x005cb780
void __fastcall FUN_005cb780(Unknown_005cb780* self, void* /*edx*/) {
    if (self->Position20 != 0 && self->Count1c > 0) {
        self->Position20 = self->Position20 - 1;
        int32_t offset = self->Position20 * 2;
        uint16_t* text = self->Text24;
        uint16_t* slot = (uint16_t*)((uint8_t*)text + offset);
        uint16_t current = *slot;
        while (current != 0) {
            offset += 2;
            *slot = *(uint16_t*)((uint8_t*)text + offset);
            text = self->Text24;
            slot = (uint16_t*)((uint8_t*)text + offset);
            current = *slot;
        }
        self->Count1c = self->Count1c - 1;
    }
}
GP4_IMPL(0x005cb780, FUN_005cb780, "thiscall(ptr[0x30]:{0x1c:i32[-2..5], 0x20:i32[0..8], 0x24:ptr[0x40]:u16{0,0,1,2,3,0x41}}) -> void")
