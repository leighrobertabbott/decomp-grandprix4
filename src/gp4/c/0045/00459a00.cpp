#include <gp4/gp4.h>

typedef void* (__stdcall *GetProcessHeapFn)();
typedef int (__stdcall *HeapFreeFn)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
// Heap block of a singly linked list: the next pointer sits at +0x68.
struct HeapNode {
    uint8_t Unknown00[0x68];
    HeapNode* Next;
};
// Object with a vtable pointer and the head of a list of heap blocks.
struct HeapList {
    const uint8_t* Vtable;
    HeapNode* First;
};
#pragma pack(pop)

// Destructor body: stores the class vtable 0x005f0584 and frees every block of the list at +4
// (HeapFree on the process heap), advancing +4 to the next block each time.
// FUNCTION: GP4 0x00459a00
void __fastcall FUN_00459a00(HeapList* self, void* /*edx*/) {
    HeapNode* node = self->First;
    self->Vtable = GP4_ARRAY(const uint8_t, 0x005f0584);
    if (node != nullptr) {
        HeapFreeFn heapFree = GP4_IAT(HeapFreeFn, 0x005f02a0);
        GetProcessHeapFn getProcessHeap = GP4_IAT(GetProcessHeapFn, 0x005f02a4);
        HeapNode* next;
        do {
            node = self->First;
            next = node->Next;
            heapFree(getProcessHeap(), 0u, node);
            self->First = next;
        } while (next != nullptr);
    }
}
GP4_IMPL(0x00459a00, FUN_00459a00, "thiscall(ptr[0x10]:{4:nptr[0x6c]:{0x68:nptr[0x6c]:{0x68:nptr[0x6c]:{0x68:u32{=0}}}}}) -> void")
