#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Node_005bab90 {
    Node_005bab90* Next00;
};
struct List_005bab90 {
    uint8_t Unknown00[0x10];
    Node_005bab90* Head10;
    uint32_t Value14;
};
#pragma pack(pop)

// Empties a singly linked list whose head pointer is at +0x10 (next pointer at +0 of each node):
// each node is unlinked, run through the empty destructor 0x005ae990 and freed with 0x00446a60.
// The head and the dword at +0x14 are cleared at the end.
// FUNCTION: GP4 0x005bab90
void __fastcall FUN_005bab90(List_005bab90* self, void* /*edx*/)
{
    Node_005bab90* node = self->Head10;
    while (node != 0) {
        self->Head10 = node->Next00;
        if (node != 0) {
            gp4::thiscall_<0x005ae990, void>(node);
            gp4::cdecl_<0x00446a60, int>(node);
        }
        node = self->Head10;
    }
    self->Head10 = 0;
    self->Value14 = 0;
}
GP4_IMPL(0x005bab90, FUN_005bab90, "thiscall(ptr[0x20]:{0x10:nptr[0x10]:{0:nptr[0x10]:{0:nptr[0x10]:{0:u32{=0}}}}}) -> void")
