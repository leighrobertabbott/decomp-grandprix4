#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Node_005b94e0 {
    uint8_t Unknown00[0x108];
    Node_005b94e0* Next108;
};
struct List_005b94e0 {
    Node_005b94e0* Head00;
};
#pragma pack(pop)

// Empties a singly linked list whose head pointer is at +0: each node (next pointer at +0x108)
// is unlinked, run through the empty destructor 0x005ae990 and freed with 0x00446a60. The head is
// cleared at the end.
// FUNCTION: GP4 0x005b94e0
void __fastcall FUN_005b94e0(List_005b94e0* self, void* /*edx*/) {
    Node_005b94e0* node;
    while ((node = self->Head00) != 0) {
        self->Head00 = node->Next108;
        gp4::thiscall_<0x005ae990, void>(node);
        gp4::cdecl_<0x00446a60, int>(node);
    }
    self->Head00 = 0;
}
GP4_IMPL(0x005b94e0, FUN_005b94e0, "thiscall(ptr[0x10]:{0:nptr[0x110]:{0x108:nptr[0x110]:{0x108:nptr[0x110]:{0x108:u32{=0}}}}}) -> void")
