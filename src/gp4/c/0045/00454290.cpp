#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00454290 {
    uint8_t Unknown00[0x1f0];
    uint8_t* Records1f0;   // array of 0x1cc-byte records
    uint8_t Unknown1f4[0x23c - 0x1f4];
    uint8_t Flag23c;
};
#pragma pack(pop)

// Calls 0x004596b0 for the record `index`; when the flag at +0x23c is set it additionally sets bit 3
// in the dword at +0x198 and bits 7 and 11 (0x880) in the dword at +0x12c of that record.
// FUNCTION: GP4 0x00454290
void __fastcall FUN_00454290(Unknown_00454290* self, void* /*edx*/, int index) {
    gp4::thiscall_<0x004596b0, void>(self, index);
    if (self->Flag23c != 0) {
        uint8_t* record = self->Records1f0 + index * 0x1cc;
        *reinterpret_cast<uint32_t*>(record + 0x198) |= 8;
        *reinterpret_cast<uint32_t*>(record + 0x12c) |= 0x880;
    }
}
GP4_IMPL(0x00454290, FUN_00454290, "thiscall(ptr[0x250]:{0x18:u32{0,1,2,3}, 0x1f0:ptr[0xe60]:bytes, 0x23c:u8{0,1}}, i32[0..7]) -> void")
