#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b8be0 {
    uint8_t* Data00;
    int32_t Value04;
    int32_t Count08;
};
#pragma pack(pop)

// Stores the two header values, allocates (count * 2 + 2) bytes with 0x00446a40 and copies that many
// bytes (a counted 16-bit string plus its terminator) from the source into the new buffer.
// FUNCTION: GP4 0x005b8be0
void __fastcall FUN_005b8be0(Unknown_005b8be0* self, void* /*edx*/, int32_t value, const uint8_t* source, int32_t count) {
    self->Value04 = value;
    self->Count08 = count;
    uint8_t* dest = (uint8_t*)gp4::cdecl_<0x00446a40, void*>(count * 2 + 2);
    self->Data00 = dest;
    uint32_t size = (uint32_t)self->Count08 * 2 + 2;
    for (uint32_t words = size >> 2; words != 0; words--) {
        *(uint32_t*)dest = *(const uint32_t*)source;
        source += 4;
        dest += 4;
    }
    for (uint32_t rest = size & 3; rest != 0; rest--) {
        *dest = *source;
        source++;
        dest++;
    }
}
GP4_IMPL(0x005b8be0, FUN_005b8be0, "thiscall(ptr[0x10]:bytes, i32, ptr[0x80]:bytes, i32[0..60]) -> void")
