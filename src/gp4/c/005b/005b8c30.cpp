#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b8c30 {
    uint16_t* Data00;
    int32_t Value04;
    int32_t Count08;
};
#pragma pack(pop)

// Stores the two header values, allocates (count * 2 + 2) bytes with 0x00446a40 and widens the count + 1
// source bytes into 16-bit words of that buffer.
// FUNCTION: GP4 0x005b8c30
void __fastcall FUN_005b8c30(Unknown_005b8c30* self, void* /*edx*/, int32_t value, const uint8_t* source, int32_t count) {
    self->Value04 = value;
    self->Count08 = count;
    self->Data00 = reinterpret_cast<uint16_t*>(gp4::cdecl_<0x00446a40, void*>(count * 2 + 2));
    int32_t index = 0;
    if (self->Count08 >= 0) {
        do {
            self->Data00[index] = source[index];
            index++;
        } while (index <= self->Count08);
    }
}
GP4_IMPL(0x005b8c30, FUN_005b8c30, "thiscall(ptr[0x10]:bytes, i32, ptr[0x80]:bytes, i32[-1..60]) -> void")
