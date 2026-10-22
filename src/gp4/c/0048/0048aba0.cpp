#include <gp4/gp4.h>

// Returns a dword from the per-index block pointed to by the record at self + index * 0x2a0 + 0x178,
// at +0x88 plus a table-selected slot.
// FUNCTION: GP4 0x0048aba0
int __fastcall FUN_0048aba0(uint8_t* self, void* /*edx*/, int index) {
    const int* slotOf = (const int*)GP4_ARRAY(int, 0x006444d0);
    const int* base = (const int*)GP4_ARRAY(int, 0x00644294);
    uint8_t* rec = self + index * 0x2a0;
    uint8_t* block = *(uint8_t**)(rec + 0x178);
    return *(int*)(block + 0x88 + base[slotOf[index]] * 4);
}
GP4_IMPL(0x0048aba0, FUN_0048aba0, "thiscall(ptr[0x1000]:u32{=0x00640000}, i32[0..3]) -> i32")
