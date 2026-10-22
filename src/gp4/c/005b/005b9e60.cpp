#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Node_005b9e60 {
    uint8_t Unknown00[0x18];
    Node_005b9e60* Next18;
};
struct List_005b9e60 {
    uint8_t Unknown00[0x24];
    Node_005b9e60* Head24;
};
#pragma pack(pop)

// Empties a singly linked list whose head pointer is at +0x24: each node (next pointer at +0x18)
// is unlinked, run through the empty destructor 0x005ae990 and freed with 0x00446a60. The head is
// cleared at the end.
// FUNCTION: GP4 0x005b9e60
void __fastcall FUN_005b9e60(List_005b9e60* self, void* /*edx*/)
{
    Node_005b9e60* node = self->Head24;
    while (node != 0) {
        self->Head24 = node->Next18;
        if (node != 0) {
            gp4::thiscall_<0x005ae990, void>(node);
            gp4::cdecl_<0x00446a60, int>(node);
        }
        node = self->Head24;
    }
    self->Head24 = 0;
}
GP4_IMPL(0x005b9e60, FUN_005b9e60, "thiscall(ptr[0x30]:{0x24:nptr[0x28]:{0x18:nptr[0x28]:{0x18:nptr[0x28]:{0x18:u32{=0}}}}}) -> void")
