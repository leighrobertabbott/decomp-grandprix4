#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef void* (__stdcall* HeapAlloc_t)(void* heap, uint32_t flags, uint32_t bytes);

#pragma pack(push, 1)
// 8-byte singly linked list node allocated from the process heap.
struct Node_00456f50 {
    int32_t Value00;
    Node_00456f50* Next04;
};
struct Inner_00456f50 {
    uint8_t Unknown00[0x28];
    uint8_t* Table28;
};
struct Owner_00456f50 {
    uint8_t Unknown00[4];
    Inner_00456f50* Inner04;
};
struct Unknown_00456f50 {
    uint8_t Unknown00[0x1ac];
    Owner_00456f50* Owner1ac;
    uint8_t Unknown1b0[0x64];
    Node_00456f50* List214;
};
#pragma pack(pop)

// Pushes `index` onto the singly linked list headed at +0x214 (an 8-byte HeapAlloc node on the process
// heap, skipped when the allocation fails). When `bump` is non-zero and index is not -1 it also
// increments the dword at the start of record `index` (0x54 bytes each) of the table reached through
// +0x1ac -> +4 -> +0x28.
// FUNCTION: GP4 0x00456f50
void __fastcall FUN_00456f50(Unknown_00456f50* self, void* /*edx*/, int32_t index, char bump) {
    Node_00456f50* node = static_cast<Node_00456f50*>(
        GP4_IAT(HeapAlloc_t, 0x005f02a8)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, 8));
    if (node != nullptr) {
        node->Value00 = index;
        node->Next04 = self->List214;
        self->List214 = node;
    }
    if (bump != 0 && index != -1) {
        int32_t* count = reinterpret_cast<int32_t*>(self->Owner1ac->Inner04->Table28 + index * 0x54);
        *count = *count + 1;
    }
}
GP4_IMPL(0x00456f50, FUN_00456f50, "thiscall(ptr[0x220]:{0x1ac:ptr[0x10]:{4:ptr[0x40]:{0x28:ptr[0x300]:bytes}}}, i32[-1..3], u32{0,0,1,2,0x100}) -> void")
