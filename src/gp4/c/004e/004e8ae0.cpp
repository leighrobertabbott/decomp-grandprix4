#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004e8ae0 {
    uint32_t Size00;
    uint8_t Unknown04[8];
    uint8_t* Buffer0c;
};
#pragma pack(pop)

// Zero-fills the buffer at +0xc for the byte count stored at +0; returns 0.
// FUNCTION: GP4 0x004e8ae0
int __fastcall FUN_004e8ae0(Unknown_004e8ae0* self, void* /*edx*/) {
    uint32_t size = self->Size00;
    uint32_t* dwords = (uint32_t*)self->Buffer0c;
    for (uint32_t n = size >> 2; n != 0; n--) {
        *dwords++ = 0;
    }
    uint8_t* bytes = (uint8_t*)dwords;
    for (uint32_t n = size & 3; n != 0; n--) {
        *bytes++ = 0;
    }
    return 0;
}
GP4_IMPL(0x004e8ae0, FUN_004e8ae0, "thiscall(ptr[0x20]:{0:u32[0..0x40], 0xc:ptr[0x48]:bytes}) -> i32")
