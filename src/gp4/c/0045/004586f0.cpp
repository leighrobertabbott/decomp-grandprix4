#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004586f0 {
    uint8_t Unknown00[0x1b0];
    uint32_t Count1b0;
    uint8_t Unknown1b4[4];
    uint8_t* Items1b8;  // array of 0x14-byte records, flag dword at +0
    uint8_t Unknown1bc[0xc];
    uint8_t* Keys1c8;   // array of 0x10-byte records, key dword at +0
};
#pragma pack(pop)

// For each of the Count1b0 entries: sets bit 0x10 of the dword at the start of the 0x14-byte record
// in Items1b8 when the dword at the start of the matching 0x10-byte record in Keys1c8 is not -1,
// clears the bit when it is -1.
// FUNCTION: GP4 0x004586f0
void __fastcall FUN_004586f0(Unknown_004586f0* self, void* /*edx*/) {
    uint32_t i = 0;
    if (self->Count1b0 > 0) {
        do {
            int32_t key = *reinterpret_cast<int32_t*>(self->Keys1c8 + i * 0x10);
            uint32_t* flags = reinterpret_cast<uint32_t*>(self->Items1b8 + i * 0x14);
            if (key == -1) {
                *flags = *flags & 0xffffffefu;
            } else {
                *flags = *flags | 0x10u;
            }
            i++;
        } while (i < self->Count1b0);
    }
}
GP4_IMPL(0x004586f0, FUN_004586f0, "thiscall(ptr[0x1d0]:{0x1b0:u32[0..6], 0x1b8:ptr[0x80]:bytes, 0x1c8:ptr[0x80]:{0*6/16:i32{-1,3,7}}}) -> void")
