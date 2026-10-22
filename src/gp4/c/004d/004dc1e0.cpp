#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004dc1e0_Node { uint32_t Next00; };
struct Unknown_004dc1e0 { Unknown_004dc1e0_Node* Head00; int32_t Unknown04; int32_t Count08; };
#pragma pack(pop)

// Pops the head of an intrusive list: advances the head to the next link,
// decrements the count at +8 and returns the old head.
// FUNCTION: GP4 0x004dc1e0
Unknown_004dc1e0_Node* __cdecl FUN_004dc1e0(Unknown_004dc1e0* list) {
    Unknown_004dc1e0_Node* old = list->Head00;
    list->Head00 = (Unknown_004dc1e0_Node*)old->Next00;
    list->Count08 = list->Count08 - 1;
    return old;
}
GP4_IMPL(0x004dc1e0, FUN_004dc1e0, "cdecl(ptr[0x10]:{0:ptr[0x10]:bytes, 8:i32[-100..100]}) -> i32")
