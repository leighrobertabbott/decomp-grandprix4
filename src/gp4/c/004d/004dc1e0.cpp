#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Node_004dc1e0 { Node_004dc1e0* Next00; };
struct Unknown_004dc1e0 { Node_004dc1e0* Head00; int32_t Unknown04; int32_t Count08; };
#pragma pack(pop)

// Pops the head of a singly linked list: head = head->next, count -= 1.
// FUNCTION: GP4 0x004dc1e0
void __cdecl FUN_004dc1e0(Unknown_004dc1e0* list) {
    list->Head00 = list->Head00->Next00;
    list->Count08 = list->Count08 - 1;
}
GP4_IMPL(0x004dc1e0, FUN_004dc1e0, "cdecl(ptr[0x400]:layout=chain_0052e1bc) -> void")
