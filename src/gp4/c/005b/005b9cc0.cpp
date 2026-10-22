#include <gp4/gp4.h>

struct Unknown_005b9cc0;

#pragma pack(push, 1)
struct Unknown_005b9cc0_Node { uint8_t Unknown00[4]; Unknown_005b9cc0_Node* Next04; };
struct Unknown_005b9cc0 { uint8_t Unknown00[0x84]; Unknown_005b9cc0_Node* Head84; };
#pragma pack(pop)

// Empties the singly linked list headed at +0x84: unlinks the first node (next pointer at +4), runs the
// empty destructor routine 0x005ae990 on it and frees it with 0x00446a60, until the head is null.
// FUNCTION: GP4 0x005b9cc0
void __fastcall FUN_005b9cc0(Unknown_005b9cc0* self, void* /*edx*/) {
    Unknown_005b9cc0_Node* node;
    while ((node = self->Head84) != nullptr) {
        self->Head84 = node->Next04;
        gp4::cdecl_<0x005ae990, void>();
        gp4::cdecl_<0x00446a60, int>(node);
    }
    self->Head84 = nullptr;
}
GP4_IMPL(0x005b9cc0, FUN_005b9cc0, "thiscall(ptr[0x90]:{0x84:nptr[0x10]:{4:nptr[0x10]:{4:nptr[0x10]:{4:u32{=0}}}}}) -> void")
