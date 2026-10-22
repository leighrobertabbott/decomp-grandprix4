#include <gp4/gp4.h>

#pragma pack(push, 1)
// 0x160-byte record of the array at 0x8943a0 (fields read by this routine only).
struct Rec_0050ba49 {
    uint8_t Unknown00[0x48];
    uint16_t Flags48;
    uint8_t Unknown4A[0x16];
    uint16_t Unknown60;
    uint16_t Unknown62;
    uint8_t Unknown64[0x24];
    uint16_t Unknown88;
    uint16_t Unknown8A;
    uint8_t Unknown8C[0x10];
    uint16_t Unknown9C;
    uint8_t Unknown9E[0x9a];
    uint16_t Unknown138;
    uint8_t Unknown13A[4];
    uint16_t Unknown13E;
    uint8_t Unknown140[0x20];
};
#pragma pack(pop)

// (value * [0x73d198] + bias) >> 15 on the 64-bit product, low 32 bits kept.
static inline int32_t Scale15_0050ba49(int32_t value, int32_t bias) {
    const int64_t p = (int64_t)value * (int64_t)GP4_GLOBAL(int32_t, 0x0073d198) + (int64_t)bias;
    return (int32_t)(p >> 15);
}

// Record `index` of 0x8943a0 (record 0 when at or past the end pointer [0x866ecc]):
// scales the words +0x60/+0x62 of the next record and of this one (+0x50 when flag word
// +0x48 has 0x200/0x100) into 0xa69878..0xa69884, then picks a 16-bit id from +0x88,
// +0x9c/+0x138 or +0x8a/+0x13e and stores (id << 16) | kind in *out (1) or -1 (0).
// FUNCTION: GP4 0x0050ba49
int __stdcall FUN_0050ba49(int index, uint32_t* out) {
    Rec_0050ba49* rec = (Rec_0050ba49*)0x008943a0 + index;
    if ((uint32_t)rec >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        rec = (Rec_0050ba49*)0x008943a0;
    }
    const Rec_0050ba49* next = rec + 1;
    uint32_t v = next->Unknown60;
    if (next->Flags48 & 0x200) v += 0x50;
    GP4_GLOBAL(int32_t, 0x00a69878) = Scale15_0050ba49(-(int32_t)v, 0x202000);
    v = next->Unknown62;
    if (next->Flags48 & 0x100) v += 0x50;
    GP4_GLOBAL(int32_t, 0x00a6987c) = Scale15_0050ba49((int32_t)v, 0x1f8000);
    v = rec->Unknown60;
    if (rec->Flags48 & 0x200) v += 0x50;
    GP4_GLOBAL(int32_t, 0x00a69880) = Scale15_0050ba49(-(int32_t)v, 0x202000);
    v = rec->Unknown62;
    if (rec->Flags48 & 0x100) v += 0x50;
    GP4_GLOBAL(int32_t, 0x00a69884) = Scale15_0050ba49((int32_t)v, 0x1f8000);

    uint32_t result;
    if (GP4_GLOBAL(int32_t, 0x00a53ad0) == 0 && GP4_GLOBAL(int32_t, 0x0064594c) <= 1 &&
        rec->Unknown88 != 0xffff) {
        result = (uint32_t)rec->Unknown88 << 16;
    } else {
        const uint16_t first = GP4_GLOBAL(int32_t, 0x00645950) == 1 ? rec->Unknown9C : rec->Unknown138;
        if (first != 0xffff) {
            result = ((uint32_t)first << 16) | 1;
        } else {
            const uint16_t second = GP4_GLOBAL(int32_t, 0x00645950) == 1 ? rec->Unknown8A : rec->Unknown13E;
            if (second == 0xffff) {
                *out = 0xffffffff;
                return 0;
            }
            result = ((uint32_t)second << 16) | 2;
        }
    }
    *out = result;
    return 1;
}
GP4_IMPL(0x0050ba49, FUN_0050ba49, "stdcall(i32[0..30], ptr[8]:bytes) -> i32; globals=layout=records_008943a0; gfields={0x0073d198:i32, 0x00a53ad0:u32{=0,1}, 0x0064594c:i32{=-1,0,1,2}, 0x00645950:u32{=0,1,2}, 0x008943e8*32/0x160:u16, 0x00894400*32/0x160:u16, 0x00894402*32/0x160:u16, 0x00894428*32/0x160:u16{0xffff,0x7fff}, 0x0089442a*32/0x160:u16{0xffff,0x7fff}, 0x0089443c*32/0x160:u16{0xffff,0x7fff}, 0x008944d8*32/0x160:u16{0xffff,0x7fff}, 0x008944de*32/0x160:u16{0xffff,0x7fff}}")
