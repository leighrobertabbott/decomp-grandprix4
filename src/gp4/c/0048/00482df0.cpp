#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00482df0 {
    uint8_t* Map00;                          // +0x00 byte map, indexed row*Pitch80 + col
    uint8_t Unknown04[0x78];
    int32_t Size7C;                          // +0x7c side of the square map at +0x40494
    int32_t Pitch80;                         // +0x80
    uint8_t Unknown84[0x10c];
    uint8_t Table190[0x100];                 // +0x190
    uint8_t Unknown290[0x100];
    uint8_t Table390[0x100];                 // +0x390
    uint8_t Unknown490[0x40494 - 0x490];
    uint8_t* Map40494;                       // +0x40494 Size7C x Size7C byte map (wraps)
    uint8_t Unknown40498[0xc];
    int32_t Blend404a4;                      // +0x404a4 blend weight out of 0x80
};
#pragma pack(pop)

// Looks up a byte in the wrapping Size7C x Size7C map (+0x40494) at (x, y) and a byte in the
// map at +0 at (row, col), maps their positive difference through the byte tables at +0x190 /
// +0x390 and blends the result with the 256-entry colour table 0xa4bfd8 (built once, while
// the flag 0x643998 is set) by the weight +0x404a4 / 0x80, returning a packed 0xAAGGGGGG colour.
// FUNCTION: GP4 0x00482df0
uint32_t __fastcall FUN_00482df0(Unknown_00482df0* self, void* /*edx*/, int x, int y, int row,
                                 int col) {
    int size = self->Size7C;
    if (y >= size) y = y % size;
    if (x >= size) x = x % size;
    uint8_t a = self->Map40494[size * y + x];
    uint8_t b = self->Map00[self->Pitch80 * row + col];
    if (GP4_GLOBAL(uint8_t, 0x00643998) != 0) {
        GP4_GLOBAL(uint8_t, 0x00643998) = 0;
        uint8_t alpha = 0xe6;
        for (int i = 0; i < 0x80; i++) {
            uint8_t v = (uint8_t)(i + i);
            if (i * 2 > 0x80) v = 0x80;
            GP4_ARRAY(uint32_t, 0x00a4bfd8)[i] = (uint32_t)v * 0x10101 - 0x1a000000;
        }
        for (int i = 0x80; i < 0x100; i++) {
            alpha--;
            GP4_ARRAY(uint32_t, 0x00a4bfd8)[i] = (uint32_t)(i & 0xff) * 0x10101 + ((uint32_t)alpha << 24);
        }
    }
    int d = (int)b - (int)a;
    if (d <= 0) d = 0;
    uint8_t c = (uint8_t)(0xff - self->Table190[d]);
    uint8_t e;
    if (c > 0xbe)
        e = self->Table390[0xff - c];
    else
        e = self->Table190[0xff - c];
    uint32_t w = (uint32_t)self->Blend404a4;
    const uint8_t* entry = (const uint8_t*)&GP4_ARRAY(uint32_t, 0x00a4bfd8)[0xff - c];
    uint32_t hi = ((entry[3] * w + e * (0x80 - w)) >> 7) & 0xff;
    uint32_t lo = ((entry[0] * w + (0x80 - w) * c) >> 7) & 0xff;
    return (hi << 24) + lo * 0x10101;
}
GP4_IMPL(0x00482df0, FUN_00482df0, "thiscall(ptr[0x40500]:{*bytes, 0:ptr[0x100]:bytes, 0x7c:i32[1..16], 0x80:i32[0..15], 0x40494:ptr[0x100]:bytes, 0x404a4:i32}, i32[0..31], i32[0..31], i32[0..15], i32[0..15]) -> u32; gfields={0x00643998:u8{=0,1}, 0x00a4bfd8*256/4:u32}")
