#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef void* (__stdcall* HeapAlloc_t)(void* heap, uint32_t flags, uint32_t bytes);

#pragma pack(push, 1)
// 8-byte singly linked list node allocated from the process heap.
struct Node_00456e40 {
    int32_t Value00;
    Node_00456e40* Next04;
};
struct Inner_00456e40 {
    uint8_t Unknown00[0x24];
    uint8_t* Table24;
};
struct Owner_00456e40 {
    uint8_t Unknown00[4];
    Inner_00456e40* Inner04;
};
struct Unknown_00456e40 {
    uint8_t Unknown00[0x1ac];
    Owner_00456e40* Owner1ac;
    uint8_t Unknown1b0[0x60];
    Node_00456e40* List210;
};
#pragma pack(pop)

// Pushes `index` onto the singly linked list headed at +0x210 (an 8-byte HeapAlloc node on the process
// heap, skipped when the allocation fails). When `bump` is non-zero and index is not -1 it also
// increments the dword at the start of record `index` (0x4c bytes each) of the table reached through
// +0x1ac -> +4 -> +0x24.
// FUNCTION: GP4 0x00456e40
void __fastcall FUN_00456e40(Unknown_00456e40* self, void* /*edx*/, int32_t index, char bump) {
    Node_00456e40* node = static_cast<Node_00456e40*>(
        GP4_IAT(HeapAlloc_t, 0x005f02a8)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, 8));
    if (node != nullptr) {
        node->Value00 = index;
        node->Next04 = self->List210;
        self->List210 = node;
    }
    if (bump != 0 && index != -1) {
        int32_t* count = reinterpret_cast<int32_t*>(self->Owner1ac->Inner04->Table24 + index * 0x4c);
        *count = *count + 1;
    }
}
GP4_IMPL(0x00456e40, FUN_00456e40, "thiscall(ptr[0x220]:{0x1ac:ptr[0x10]:{4:ptr[0x40]:{0x24:ptr[0x300]:bytes}}}, i32[-1..3], u32{0,0,1,2,0x100}) -> void")
