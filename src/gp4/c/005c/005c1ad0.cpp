#include <gp4/gp4.h>

struct Unknown_005c1ad0;
struct Unknown_005c1ad0_Node;

typedef void (__fastcall *Unknown_005c1ad0_Slot70)(Unknown_005c1ad0_Node* self, void* edx);

#pragma pack(push, 1)
struct Unknown_005c1ad0_Node {
    uint32_t* Vtable00;
    uint8_t Unknown04[0x2c];
    int32_t State30;
    uint8_t Unknown34[0xc8];
    uint32_t Link0fc;
};
struct Unknown_005c1ad0 {
    uint8_t Unknown00[4];
    Unknown_005c1ad0_Node* Node04;
};
#pragma pack(pop)

// Follows the chain of objects whose node is in state 4 (each link is an index into the pointer table
// at 0x00b35afc; a zero link or a missing node ends the walk); on the first node that is not in state 4,
// calls its virtual slot 0x70 unless the global byte 0x009ed5b0 is set.
// FUNCTION: GP4 0x005c1ad0
void __fastcall FUN_005c1ad0(Unknown_005c1ad0* self, void* /*edx*/) {
    Unknown_005c1ad0_Node* node = self->Node04;
    if (node == 0) {
        return;
    }
    while (node->State30 == 4) {
        const uint32_t link = node->Link0fc;
        if (link == 0) {
            return;
        }
        self = GP4_ARRAY(Unknown_005c1ad0*, 0x00b35afc)[link];
        node = self->Node04;
        if (node == 0) {
            return;
        }
    }
    if (GP4_GLOBAL(uint8_t, 0x009ed5b0) == 0) {
        Unknown_005c1ad0_Node* target = self->Node04;
        Unknown_005c1ad0_Slot70 method = (Unknown_005c1ad0_Slot70)target->Vtable00[0x70 / 4];
        method(target, 0);
    }
}
GP4_IMPL(0x005c1ad0, FUN_005c1ad0, "thiscall(ptr[0x10]:{4:nptr[0x1a0]:{0:ptr[0xc0]:{0x70:u32{=0x005b3d70,0x005b3d50}}, 0x30:u32{=4,5}, 0xfc:u32{=0,1,2,3}}}) -> void; gfields={0x009ed5b0:u8{=0,1}, 0x00b35afc*4/4:ptr[0x10]:{4:nptr[0x1a0]:{0:ptr[0xc0]:{0x70:u32{=0x005b3d70,0x005b3d50}}, 0x30:u32{=4,5}, 0xfc:u32{=0}}}}")
