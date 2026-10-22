#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Parser_0045a1d0 {
    uint8_t Unknown00[8];
    const char* Cursor08;
    uint8_t Unknown0C[4];
    uint8_t Flag10;
};
#pragma pack(pop)

// Advances the cursor at +8 past the next ',' or ';' (whichever comes first) unless the flag at +0x10 is 1.
// If neither separator is found the cursor is just incremented.
// FUNCTION: GP4 0x0045a1d0
void __fastcall FUN_0045a1d0(Parser_0045a1d0* self, void* /*edx*/) {
    if (self->Flag10 == 1) {
        return;
    }
    const char* comma = gp4::cdecl_<0x005db7d0, const char*>(self->Cursor08, 0x2c);
    const char* semi = gp4::cdecl_<0x005db7d0, const char*>(self->Cursor08, 0x3b);
    if (comma == 0) {
        if (semi != 0) {
            self->Cursor08 = semi;
        }
    } else if (semi == 0 || comma < semi) {
        self->Cursor08 = comma;
    } else {
        self->Cursor08 = semi;
    }
    self->Cursor08 = self->Cursor08 + 1;
}
GP4_IMPL(0x0045a1d0, FUN_0045a1d0, "thiscall(ptr[0x20]:{8:ptr[0x40]:u8{0x2c,0x3b,0x41,0x00}, 0x10:u8{0,1,2}}) -> void")
