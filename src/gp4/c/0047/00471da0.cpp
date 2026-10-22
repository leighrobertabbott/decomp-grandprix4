#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_00471da0 {
    uint32_t Unknown00;
    uint8_t* Data04;  // points just past a one-byte reference count (count at Data04[-1])
    uint32_t Unknown08;
    uint32_t Unknown0c;
};
#pragma pack(pop)

// Releases a reference to the counted block whose data pointer is stored at +4: the block starts one
// byte before the data and that byte is the count; a count of 0 or 0xff frees the block with
// HeapFree(GetProcessHeap(), 0, ...), any other count is decremented. Then clears +4, +8 and +0xc.
// FUNCTION: GP4 0x00471da0
void __fastcall FUN_00471da0(Unknown_00471da0* self, void* /*edx*/) {
    uint8_t* data = self->Data04;
    if (data != nullptr) {
        uint8_t count = data[-1];
        uint8_t* block = data - 1;
        if (count == 0 || count == 0xff) {
            GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, block);
        } else {
            *block = static_cast<uint8_t>(count - 1);
        }
    }
    self->Data04 = nullptr;
    self->Unknown08 = 0;
    self->Unknown0c = 0;
}
GP4_IMPL(0x00471da0, FUN_00471da0, "thiscall(ptr[0x20]:u32{=0,0x00a565b1,0x00a565b5,0x005f0cad}) -> void")
