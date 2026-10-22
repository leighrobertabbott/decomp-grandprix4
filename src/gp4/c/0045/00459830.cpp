#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
// Heap block of a singly linked list: the next pointer sits at +0x44.
struct ListNode {
    uint8_t Unknown00[0x44];
    ListNode* Next44;
};
struct Unknown_00459830 {
    uint32_t Vtable00;
    ListNode* First04;
};
#pragma pack(pop)

// Scalar deleting destructor: stores the class vtable 0x005f0580 at +0, frees every block of the
// list headed at +4 (HeapFree on the process heap), then frees the object itself when bit 0 of
// flags is set. Returns this.
// FUNCTION: GP4 0x00459830
Unknown_00459830* __fastcall FUN_00459830(Unknown_00459830* self, void* /*edx*/, uint32_t flags) {
    HeapFree_t heapFree = GP4_IAT(HeapFree_t, 0x005f02a0);
    GetProcessHeap_t getProcessHeap = GP4_IAT(GetProcessHeap_t, 0x005f02a4);
    ListNode* node = self->First04;
    self->Vtable00 = 0x005f0580;
    if (node != nullptr) {
        ListNode* next;
        do {
            node = self->First04;
            next = node->Next44;
            heapFree(getProcessHeap(), 0, node);
            self->First04 = next;
        } while (next != nullptr);
    }
    if (flags & 1) {
        heapFree(getProcessHeap(), 0, self);
    }
    return self;
}
GP4_IMPL(0x00459830, FUN_00459830, "thiscall(ptr[0x10]:layout=lst_00459880, u32) -> i32")
