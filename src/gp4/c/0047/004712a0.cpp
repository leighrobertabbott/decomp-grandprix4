#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004712a0 {
    uint8_t Flag00;
    uint8_t Unknown01[7];
    void* File08;      // CRT FILE
    void* Buffer0c;    // process-heap block (non-null while the stream is open)
};
#pragma pack(pop)

// Rewinds the stream: when the buffer at +0xc is set, seeks the CRT FILE at +8 to offset 0 from
// the start (fseek 0x005dcb47) and clears the flag byte at +0. Returns the fseek result, or 0.
// FUNCTION: GP4 0x004712a0
int __fastcall FUN_004712a0(Unknown_004712a0* self, void* /*edx*/) {
    int result = 0;
    if (self->Buffer0c != nullptr) {
        result = gp4::cdecl_<0x005dcb47, int>(self->File08, 0, 0);
        self->Flag00 = 0;
    }
    return result;
}
GP4_IMPL(0x004712a0, FUN_004712a0, "thiscall(ptr[0x20]:{0:u8, 8:ptr[0x40]:bytes, 0xc:u32{0,1,2}}) -> i32")
