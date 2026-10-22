#include <gp4/gp4.h>

typedef void*(__stdcall* GlobalFree_t)(void* mem);

// Frees the global memory block whose handle is stored in 0x00b1b86c, if any; returns GlobalFree's
// result (0 when the global is null).
// FUNCTION: GP4 0x00506c20
int __cdecl FUN_00506c20() {
    void* block = GP4_GLOBAL(void*, 0x00b1b86c);
    if (block != nullptr) {
        return static_cast<int>(reinterpret_cast<intptr_t>(GP4_IAT(GlobalFree_t, 0x005f0244)(block)));
    }
    return 0;
}
GP4_IMPL(0x00506c20, FUN_00506c20, "cdecl() -> i32; globals=fuzz:i32")
