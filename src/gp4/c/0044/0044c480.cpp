#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044c480 {
    uint8_t Unknown00[8];
    uint8_t** Cell08;     // *Cell08 -> array of 0x14-byte records (flag dword at +0)
    uint32_t Index0c;
};
#pragma pack(pop)

// Sets (set != 0) or clears (set == 0) the bits of `mask` in the flag dword of record Index0c
// (0x14-byte records starting at **Cell08).
// FUNCTION: GP4 0x0044c480
void __fastcall FUN_0044c480(Unknown_0044c480* self, void* /*edx*/, uint32_t mask, char set) {
    if (set != 0) {
        uint32_t* rec = reinterpret_cast<uint32_t*>(*self->Cell08 + self->Index0c * 0x14);
        *rec = *rec | mask;
        return;
    }
    uint32_t* rec = reinterpret_cast<uint32_t*>(*self->Cell08 + self->Index0c * 0x14);
    *rec = *rec & ~mask;
}
GP4_IMPL(0x0044c480, FUN_0044c480, "thiscall(ptr[0x34]:layout=elem_0044c480, u32, u8) -> void")
