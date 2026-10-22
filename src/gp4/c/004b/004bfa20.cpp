#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Record_004bfa20 { uint32_t Kind00; uint8_t Unknown04[0x94]; };
struct Unknown_004bfa20 {
    uint8_t Unknown00[0xc];
    int32_t Index0c;
    uint8_t Unknown10[0x70];
    Record_004bfa20 Records80[4];
};
#pragma pack(pop)

// True when the record selected by the index at +0xc (0x98-byte records starting at +0x80) does not
// hold 0x20000000 in its first dword. The stack argument is popped but unused.
// FUNCTION: GP4 0x004bfa20
uint8_t __fastcall FUN_004bfa20(Unknown_004bfa20* self, void* /*edx*/, int /*unused*/) {
    return self->Records80[self->Index0c].Kind00 != 0x20000000u;
}
GP4_IMPL(0x004bfa20, FUN_004bfa20, "thiscall(ptr[0x300]:{0xc:u32[0..3], 0x80*4/0x98:u32{0x20000000,0,7}}, i32) -> u8")
