#include <gp4/gp4.h>

#pragma pack(push, 1)
// One displayed character: a flag per decimal digit 0..9 and a minus flag.
struct DigitFlags_00470630 {
    uint8_t Digit[10];
    uint8_t Minus;
};
struct Unknown_00470630 {
    uint8_t Unknown000[0x9e0];
    uint8_t Flag9e0;
    uint8_t Flag9e1;
    union {
        DigitFlags_00470630 Chars[6];        // +0x9e2 .. +0xa23
        struct { uint32_t Dwords[16]; uint16_t Word; } Raw;
    } Flags9e2;
    uint8_t Unknowna24[0xc50 - 0xa24];
    int32_t Char0c50[6];                     // +0xc50 .. +0xc64 the six characters
};
#pragma pack(pop)

// Stores six characters (+0xc50..+0xc64) and rebuilds their flag blocks at +0x9e2 (11 bytes each:
// one flag per digit '0'..'9', then '-'). A first character of -1 leaves everything cleared;
// otherwise +0x9e0/+0x9e1 are set, '-' sets the minus flag and any character other than ' '
// and '-' sets the flag of digit (c - '0') % 10.
// FUNCTION: GP4 0x00470630
void __fastcall FUN_00470630(Unknown_00470630* self, void* /*edx*/, int c0, int c1, int c2, int c3,
                             int c4, int c5) {
    self->Char0c50[0] = c0;
    self->Char0c50[1] = c1;
    self->Char0c50[3] = c3;
    self->Char0c50[2] = c2;
    self->Char0c50[4] = c4;
    self->Flag9e0 = 0;
    self->Flag9e1 = 0;
    self->Char0c50[5] = c5;
    for (int i = 0; i < 16; i++) self->Flags9e2.Raw.Dwords[i] = 0;
    self->Flags9e2.Raw.Word = 0;
    int first = self->Char0c50[0];
    if (first == -1) return;
    self->Flag9e0 = 1;
    self->Flag9e1 = 1;
    for (int i = 0; i < 6; i++) {
        if ((char)self->Char0c50[i] == '-') self->Flags9e2.Chars[i].Minus = 1;
    }
    for (int i = 0; i < 6; i++) {
        char ch = (char)self->Char0c50[i];
        if (ch != ' ' && ch != '-') self->Flags9e2.Chars[i].Digit[(self->Char0c50[i] - 0x30) % 10] = 1;
    }
}
GP4_IMPL(0x00470630, FUN_00470630, "thiscall(ptr[0xc70]:bytes, i32{=-1,0x20,0x2d,0x30,0x31,0x35,0x39,0x41,0x2f,0x12d,-0x20}, i32{=0x20,0x2d,0x30,0x31,0x35,0x39,0x41,0x2f,0x12d}, i32{=0x20,0x2d,0x30,0x31,0x35,0x39,0x41,0x2f,0x12d}, i32{=0x20,0x2d,0x30,0x31,0x35,0x39,0x41,0x2f,0x12d}, i32{=0x20,0x2d,0x30,0x31,0x35,0x39,0x41,0x2f,0x12d}, i32{=0x20,0x2d,0x30,0x31,0x35,0x39,0x41,0x2f,0x12d}) -> void")
