#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b9d80_Node {
    int32_t Key00;
    Unknown_005b9d80_Node* Next04;
};
struct Unknown_005b9d80 {
    uint8_t Unknown00[0x84];
    Unknown_005b9d80_Node* Head84;
};
#pragma pack(pop)

// Walks the list at +0x84 and returns the key of the node with the smallest (value - key)
// that is below 99999 (the first such node wins ties); 0 when the list is empty or no node
// qualifies.
// FUNCTION: GP4 0x005b9d80
int32_t __fastcall FUN_005b9d80(Unknown_005b9d80* self, void* /*edx*/, int32_t value) {
    int32_t best = 0;
    int32_t bestDiff = 99999;
    for (Unknown_005b9d80_Node* node = self->Head84; node != 0; node = node->Next04) {
        int32_t diff = value - node->Key00;
        if (diff < bestDiff) {
            best = node->Key00;
            bestDiff = diff;
        }
    }
    return best;
}
GP4_IMPL(0x005b9d80, FUN_005b9d80, "thiscall(ptr[0x88]:{0x84:nptr[8]:{0:i32[-200000..200000], 4:nptr[8]:{0:i32[-200000..200000], 4:nptr[8]:{0:i32[-200000..200000], 4:nptr[8]:{0:i32[-200000..200000], 4:u32{=0}}}}}}, i32[-200000..200000]) -> i32")
