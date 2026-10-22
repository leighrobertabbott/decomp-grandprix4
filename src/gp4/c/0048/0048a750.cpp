#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0048a750 {
    uint8_t Unknown00[0x30];
    uint32_t Words30[25];
    uint8_t Unknown94[0x4f];
    uint8_t Bytes0e3[26];
    uint8_t Unknown0fd[3];
    uint32_t Words100[25];
};
#pragma pack(pop)

// Copies the pending staged tables (25 dwords from 0x00a4d460 to +0x30, 25 bytes from 0x00a4d42c to
// +0xe4, 25 dwords from 0x00a4d3c8 to +0x100) into the object when the "pending" byte 0x00a4d694 is
// set, then clears that byte.
// FUNCTION: GP4 0x0048a750
void __fastcall FUN_0048a750(Unknown_0048a750* self, void* /*edx*/) {
    if (GP4_GLOBAL(uint8_t, 0x00a4d694) != 0) {
        int i = 0;
        do {
            self->Words30[i] = GP4_ARRAY(uint32_t, 0x00a4d460)[i];
            int n = i + 1;
            self->Bytes0e3[n] = GP4_ARRAY(uint8_t, 0x00a4d42b)[n];
            self->Words100[i] = GP4_ARRAY(uint32_t, 0x00a4d3c4)[n];
            i = n;
        } while (i < 0x19);
        GP4_GLOBAL(uint8_t, 0x00a4d694) = 0;
    }
}
GP4_IMPL(0x0048a750, FUN_0048a750, "thiscall(ptr[0x170]:bytes) -> void; globals=fuzz:i32")
