#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00457200 {
    uint8_t Unknown00[0x1b0];
    uint32_t Count1b0;
    uint8_t Unknown1b4[0x1c];
    uint8_t* Records1d0;  // array of 0x88-byte records, a name string at +0
};
#pragma pack(pop)

// Index of the first of the Count1b0 records (0x88 bytes each, at +0x1d0) whose leading string equals
// the given string ignoring case (CRT _stricmp 0x005dc930), or 0xffffffff when none does.
// FUNCTION: GP4 0x00457200
uint32_t __fastcall FUN_00457200(Unknown_00457200* self, void* /*edx*/, const char* name) {
    uint32_t i = 0;
    if (self->Count1b0 != 0) {
        uint32_t offset = 0;
        do {
            if (gp4::cdecl_<0x005dc930, int>(self->Records1d0 + offset, name) == 0) {
                return i;
            }
            i++;
            offset += 0x88;
        } while (i < self->Count1b0);
    }
    return 0xffffffffu;
}
GP4_IMPL(0x00457200, FUN_00457200, "thiscall(ptr[0x1e0]:{0x1b0:u32[0..4], 0x1d0:ptr[0x440]:u8{0x41,0x61,0x42,0x62,0,0}}, ptr[0x20]:u8{0x41,0x61,0x42,0x62,0,0}) -> u32")
