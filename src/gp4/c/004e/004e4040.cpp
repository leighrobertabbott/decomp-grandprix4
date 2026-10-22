#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_004e4040 {
    int32_t Count00;
    int32_t Count04;
    uint8_t Unknown08[0x18];
    void** Table20;
};
#pragma pack(pop)

// When `keep` is zero, frees every heap block in the pointer table at +0x20 (HeapFree on the process
// heap) and clears its slot, for indices 1 .. Count00-1 except the index Count04-1.
// FUNCTION: GP4 0x004e4040
void __fastcall FUN_004e4040(Unknown_004e4040* self, void* /*edx*/, char keep) {
    if (keep == 0) {
        int32_t i = 0;
        if (self->Count00 > 0) {
            HeapFree_t heapFree = GP4_IAT(HeapFree_t, 0x005f02a0);
            GetProcessHeap_t getProcessHeap = GP4_IAT(GetProcessHeap_t, 0x005f02a4);
            do {
                if (i != 0 && i != self->Count04 - 1) {
                    void* block = self->Table20[i];
                    if (block != nullptr) {
                        heapFree(getProcessHeap(), 0, block);
                        self->Table20[i] = nullptr;
                    }
                }
                i++;
            } while (i < self->Count00);
        }
    }
}
GP4_IMPL(0x004e4040, FUN_004e4040, "thiscall(ptr[0x40]:{0:i32[-1..8], 4:i32[0..9], 0x20:ptr[0x40]:u32{0,0x1000,0x2000}}, u32{0,0,1,2,0x100}) -> void")
