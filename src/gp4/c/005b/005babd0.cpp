#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Node_005babd0 {
    Node_005babd0* Next00;
};
struct List_005babd0 {
    uint8_t Unknown00[0xc];
    uint32_t Value0c;
    Node_005babd0* Head10;
    uint32_t Value14;
};
#pragma pack(pop)

// Walks the singly linked list headed at +0x10 (next pointer at +0 of each node): the next link is
// read first, then the node is run through the empty destructor 0x005ae990 and freed with
// 0x00446a60. Afterwards the head and the dwords at +0x14 and +0xc are cleared.
// FUNCTION: GP4 0x005babd0
void __fastcall FUN_005babd0(List_005babd0* self, void* /*edx*/)
{
    Node_005babd0* next = self->Head10;
    while (next != 0) {
        Node_005babd0* node = next;
        next = node->Next00;
        if (node != 0) {
            gp4::thiscall_<0x005ae990, void>(node);
            gp4::cdecl_<0x00446a60, int>(node);
        }
    }
    self->Head10 = 0;
    self->Value14 = 0;
    self->Value0c = 0;
}
GP4_IMPL(0x005babd0, FUN_005babd0, "thiscall(ptr[0x20]:{0x10:nptr[0x10]:{0:nptr[0x10]:{0:nptr[0x10]:{0:u32{=0}}}}}) -> void")
