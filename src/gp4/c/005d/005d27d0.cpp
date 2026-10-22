#include <gp4/gp4.h>

// Abstract view of the object: only the call through vtable slot 1 (+4) matters here.
struct Object_005d27d0 {
    virtual void Slot00() = 0;
    virtual void Slot01(int32_t first, int32_t second, int32_t third) = 0;
};

// Calls virtual slot 1 (vtable +4) of the object with its two stack arguments (raw dwords, floats
// in the callers) and a trailing 0: `mov eax,[ecx]; push 0; push b; push a; call [eax+4]; ret 8`.
// FUNCTION: GP4 0x005d27d0
void __fastcall FUN_005d27d0(Object_005d27d0* self, void* /*edx*/, int32_t first, int32_t second) {
    self->Slot01(first, second, 0);
}
GP4_IMPL(0x005d27d0, FUN_005d27d0, "thiscall(ptr[0x100]:{0:ptr[0x10]:{4:u32{=0x005bc1f0}}}, i32, i32) -> void")
