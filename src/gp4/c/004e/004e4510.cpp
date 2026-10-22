#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004e4510 {
    uint8_t Unknown00[4];
    uint32_t* Items04;
    uint8_t Unknown08[4];
    int32_t Count0c;
};
#pragma pack(pop)

// Reads element `index` of a dword vector (items at +4, count at +0xc) into *out.
// Returns 0 on success, 4 when index > count - 1 (signed), 3 when the vector is empty.
// FUNCTION: GP4 0x004e4510
int __fastcall FUN_004e4510(Unknown_004e4510* self, void* /*edx*/, int index, uint32_t* out) {
    if (index > self->Count0c - 1) {
        return 4;
    }
    if (self->Count0c != 0) {
        *out = self->Items04[index];
        return 0;
    }
    return 3;
}
GP4_IMPL(0x004e4510, FUN_004e4510, "thiscall(ptr[0x10]:layout=vec_004e4510, i32[-2..22], ptr[4]:bytes) -> i32")
