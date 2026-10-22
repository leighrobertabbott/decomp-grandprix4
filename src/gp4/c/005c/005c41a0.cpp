#include <gp4/gp4.h>

struct Unknown_005c41a0;

#pragma pack(push, 1)
struct Unknown_005c41a0_Node { uint8_t Unknown00[8]; Unknown_005c41a0_Node* Next08; };
struct Unknown_005c41a0 {
    uint8_t Unknown00[8];
    Unknown_005c41a0_Node* Head08;
    uint8_t Unknown0c[4];
    uint8_t Block10[0x2c];
    int32_t Value3c;
};
#pragma pack(pop)

// Frees the singly linked list headed at +8 (next pointer at +8 of each node: the empty destructor routine
// 0x005ae990, then the free routine 0x00446a60), runs the reset routine 0x005bca90 on the sub-object at +0x10,
// sets the dword at +0x3c to -1 and clears the head.
// FUNCTION: GP4 0x005c41a0
void __fastcall FUN_005c41a0(Unknown_005c41a0* self, void* /*edx*/) {
    Unknown_005c41a0_Node* node = self->Head08;
    while (node != nullptr) {
        Unknown_005c41a0_Node* next = node->Next08;
        gp4::cdecl_<0x005ae990, void>();
        gp4::cdecl_<0x00446a60, int>(node);
        node = next;
    }
    gp4::thiscall_<0x005bca90, void>(self->Block10);
    self->Value3c = -1;
    self->Head08 = nullptr;
}
GP4_IMPL(0x005c41a0, FUN_005c41a0, "thiscall(ptr[0x60]:{8:nptr[0x10]:{8:nptr[0x10]:{8:nptr[0x10]:{8:u32{=0}}}}, 0x30:nptr[16]:com=IDirect3DVertexBuffer8}) -> void; globals=fuzz:i32; gfields={0x00b2e808:ptr[16]:com=IDirect3DDevice8}")
