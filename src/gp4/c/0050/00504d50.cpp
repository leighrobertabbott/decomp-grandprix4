#include <gp4/gp4.h>

#pragma pack(push, 1)
// Sort record: an unsigned primary key at +0, an unsigned secondary key at +4 and a signed priority at +0xc.
struct Unknown_00504d50 {
    uint32_t Key00;
    uint32_t Key04;
    uint32_t Unknown08;
    int32_t Priority0c;
};
#pragma pack(pop)

// Comparator for two records: the larger signed value at +0xc sorts first (-1 when a's is larger, 1 when b's
// is), ties are broken by the unsigned dword at +0 (smaller first), then by the unsigned dword at +4.
// Returns -1, 0 or 1.
// FUNCTION: GP4 0x00504d50
int __cdecl FUN_00504d50(const Unknown_00504d50* a, const Unknown_00504d50* b) {
    if (a->Priority0c > b->Priority0c) {
        return -1;
    }
    if (a->Priority0c < b->Priority0c) {
        return 1;
    }
    if (a->Key00 < b->Key00) {
        return -1;
    }
    if (a->Key00 > b->Key00) {
        return 1;
    }
    if (b->Key04 > a->Key04) {
        return -1;
    }
    return b->Key04 < a->Key04 ? 1 : 0;
}
GP4_IMPL(0x00504d50, FUN_00504d50, "cdecl(ptr[0x10]:{0:u32{=0,1,0x80000000}, 4:u32{=0,1,0x80000000}, 0xc:i32{=-1,0,1}}, ptr[0x10]:{0:u32{=0,1,0x80000000}, 4:u32{=0,1,0x80000000}, 0xc:i32{=-1,0,1}}) -> i32")
