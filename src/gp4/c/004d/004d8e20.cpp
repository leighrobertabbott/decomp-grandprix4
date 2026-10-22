#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d8e20_Mode { uint8_t Unknown00[0x183]; uint8_t Flag183; };
struct Unknown_004d8e20_Rec { uint8_t Enabled0; uint8_t Enabled1; uint8_t Bits2; uint8_t Unknown03[0x9d]; };
struct Unknown_004d8e20 {
    uint8_t Unknown00[4];
    Unknown_004d8e20_Mode* Mode04;
    uint8_t Unknown08[0x40];
    Unknown_004d8e20_Rec* Recs48;
};
#pragma pack(pop)

// For each of the 22 objects in the pointer table 0xa51fd8, when its 0xa0-byte record in the
// array at self+0x48 has both bytes 0 and 1 set, copies bits of record byte 2 into byte flags of
// the object (bits 0..3 into four flags whose order depends on the mode byte self->+4->+0x183,
// bit 4 into five flags, bit 5 into four).
// FUNCTION: GP4 0x004d8e20
void __fastcall FUN_004d8e20(Unknown_004d8e20* self, void* /*edx*/) {
    uint8_t** obj = GP4_ARRAY(uint8_t*, 0x00a51fd8);
    int off = 0;
    for (; obj < GP4_ARRAY(uint8_t*, 0x00a52030); obj++, off += 0xa0) {
        const uint8_t mode = self->Mode04->Flag183;
        uint8_t* rec = (uint8_t*)self->Recs48 + off;
        const uint8_t en1 = rec[1];
        if (mode != 0) {
            if (en1 == 0 || rec[0] == 0)
                continue;
            (*obj)[0x3754] = rec[2] & 1;
            (*obj)[0x41d4] = (((uint8_t*)self->Recs48)[off + 2] >> 1) & 1;
            (*obj)[0x3c94] = (((uint8_t*)self->Recs48)[off + 2] >> 2) & 1;
            (*obj)[0x3214] = (((uint8_t*)self->Recs48)[off + 2] >> 3) & 1;
        } else {
            if (en1 == 0 || rec[0] == 0)
                continue;
            (*obj)[0x3214] = rec[2] & 1;
            (*obj)[0x3c94] = (((uint8_t*)self->Recs48)[off + 2] >> 1) & 1;
            (*obj)[0x41d4] = (((uint8_t*)self->Recs48)[off + 2] >> 2) & 1;
            (*obj)[0x3754] = (((uint8_t*)self->Recs48)[off + 2] >> 3) & 1;
        }
        (*obj)[0x2254] = (((uint8_t*)self->Recs48)[off + 2] >> 4) & 1;
        (*obj)[0x24f4] = (((uint8_t*)self->Recs48)[off + 2] >> 4) & 1;
        (*obj)[0x2794] = (((uint8_t*)self->Recs48)[off + 2] >> 4) & 1;
        (*obj)[0x2a34] = (((uint8_t*)self->Recs48)[off + 2] >> 4) & 1;
        (*obj)[0x2cd4] = (((uint8_t*)self->Recs48)[off + 2] >> 4) & 1;
        (*obj)[0x17d4] = (((uint8_t*)self->Recs48)[off + 2] >> 5) & 1;
        (*obj)[0x1a74] = (((uint8_t*)self->Recs48)[off + 2] >> 5) & 1;
        (*obj)[0x1d14] = (((uint8_t*)self->Recs48)[off + 2] >> 5) & 1;
        (*obj)[0x1fb4] = (((uint8_t*)self->Recs48)[off + 2] >> 5) & 1;
    }
}
GP4_IMPL(0x004d8e20, FUN_004d8e20, "thiscall(ptr[0x50]:{4:ptr[0x188]:{0x183:u8{=0,1,0x80}}, 0x48:ptr[0xdc8]:u8{=0,1,0x15,0x2a,0x3f,0xff}}) -> void; gfields={0x00a51fd8*22/4:ptr[0x41d8]:bytes}")
