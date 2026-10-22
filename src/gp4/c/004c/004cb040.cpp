#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004cb040 { uint8_t Unknown00[8]; void* Block08; };
#pragma pack(pop)

// If the object holds a block pointer at +8, releases it through FUN_004e44b0 (the original reaches
// it by a tail jump, so the callee's eax stays in eax; with a null pointer it just returns).
// FUNCTION: GP4 0x004cb040
void __fastcall FUN_004cb040(Unknown_004cb040* self, void* /*edx*/) {
    void* block = self->Block08;
    if (block != nullptr) {
        gp4::thiscall_<0x004e44b0, int>(block);
    }
}
GP4_IMPL(0x004cb040, FUN_004cb040, "thiscall(ptr[0x10]:{8:nptr[0x10]:{4:ptr[0x40]:{0*16/4:nptr[0x10]:bytes}, 8:i32[-1..8]}}) -> void")
