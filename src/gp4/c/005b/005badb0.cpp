#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Node_005badb0 {
    Node_005badb0* Next00;
};
struct List_005badb0 {
    Node_005badb0* Head00;
};
#pragma pack(pop)

// Empties a singly linked list whose head pointer is at +0 (next pointer at +0 of each node):
// each node is unlinked, run through 0x005bab90 (which empties the node's own inner list) and
// freed with 0x00446a60. The head is cleared at the end.
// FUNCTION: GP4 0x005badb0
void __fastcall FUN_005badb0(List_005badb0* self, void* /*edx*/)
{
    Node_005badb0* node = self->Head00;
    while (node != 0) {
        self->Head00 = node->Next00;
        if (node != 0) {
            gp4::thiscall_<0x005bab90, void>(node);
            gp4::cdecl_<0x00446a60, int>(node);
        }
        node = self->Head00;
    }
    self->Head00 = 0;
}
GP4_IMPL(0x005badb0, FUN_005badb0, "thiscall(ptr[0x10]:{0:nptr[0x20]:{0:nptr[0x20]:{0:nptr[0x20]:{0:u32{=0}, 0x10:nptr[0x10]:{0:u32{=0}}}, 0x10:nptr[0x10]:{0:u32{=0}}}, 0x10:nptr[0x10]:{0:u32{=0}}}}) -> void")
