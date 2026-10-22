#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Node_00464180 { uint8_t Unknown00[0x24]; Node_00464180* Next24; uint8_t Unknown28[0x18]; };
struct List_00464180 { Node_00464180* Head00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Pushes `node` onto the singly linked list whose head is at +0 of the object
// (node->+0x24 = old head; head = node). Returns the node.
// FUNCTION: GP4 0x00464180
Node_00464180* __fastcall FUN_00464180(List_00464180* self, void* /*edx*/, Node_00464180* node) {
    node->Next24 = self->Head00;
    self->Head00 = node;
    return node;
}
GP4_IMPL(0x00464180, FUN_00464180, "thiscall(ptr[0x100]:bytes, ptr[0x50]:bytes) -> i32")
