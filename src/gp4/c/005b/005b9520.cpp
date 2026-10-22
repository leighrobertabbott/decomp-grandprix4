#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b9520_Node {
    int32_t Key00;
    int32_t Value04;
    uint8_t Unknown08[0x100];
    Unknown_005b9520_Node* Next108;
};
struct Unknown_005b9520 {
    Unknown_005b9520_Node* Head00;
    Unknown_005b9520_Node* Cursor04;
};
#pragma pack(pop)

// Walks the singly linked list at +0 (next link at node+0x108) for the first node whose key
// (node+0) equals the argument, leaving the cursor at +4 on it (null when missing); returns
// the node's value at +4, or 0.
// FUNCTION: GP4 0x005b9520
int32_t __fastcall FUN_005b9520(Unknown_005b9520* self, void* /*edx*/, int32_t key) {
    int32_t result = 0;
    self->Cursor04 = self->Head00;
    while (self->Cursor04 != 0 && self->Cursor04->Key00 != key) {
        self->Cursor04 = self->Cursor04->Next108;
    }
    if (self->Cursor04 != 0) {
        result = self->Cursor04->Value04;
    }
    return result;
}
GP4_IMPL(0x005b9520, FUN_005b9520, "thiscall(ptr[0x10]:{0:nptr[0x10c]:{0:u32{=1,2,3,4}, 4:u32, 0x108:nptr[0x10c]:{0:u32{=1,2,3,4}, 4:u32, 0x108:nptr[0x10c]:{0:u32{=1,2,3,4}, 4:u32, 0x108:nptr[0x10c]:{0:u32{=1,2,3,4}, 4:u32, 0x108:u32{=0}}}}}}, i32{=1,2,3,4,9}) -> i32")
