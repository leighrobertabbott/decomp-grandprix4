#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046bd40 {
    uint8_t Unknown00[4];
    int32_t Value04;
    int32_t Value08;
    int32_t Value0c;
    uint8_t Unknown10[0x1c];
    uint32_t Time2c;
};
#pragma pack(pop)

// Formats a lap time into the caller's buffer through the CRT sprintf (0x005db088): "%01d:%02d.%03d"
// from the dwords at +0xc, +8 and +4, or the placeholder "-:--.---" when the time at +0x2c is zero or
// above 600000. Returns sprintf's result.
// FUNCTION: GP4 0x0046bd40
int __fastcall FUN_0046bd40(Unknown_0046bd40* self, void* /*edx*/, char* buffer) {
    if (self->Time2c > 600000u) {
        return gp4::cdecl_<0x005db088, int>(buffer, GP4_ARRAY(const char, 0x0063e910), GP4_ARRAY(const char, 0x006408b4));
    }
    if (self->Time2c == 0) {
        return gp4::cdecl_<0x005db088, int>(buffer, GP4_ARRAY(const char, 0x0063e910), GP4_ARRAY(const char, 0x006408b4));
    }
    return gp4::cdecl_<0x005db088, int>(buffer, GP4_ARRAY(const char, 0x006408a4), self->Value0c, self->Value08, self->Value04);
}
GP4_IMPL(0x0046bd40, FUN_0046bd40, "thiscall(ptr[0x40]:{4:i32[0..999], 8:i32[0..59], 0xc:i32[0..9], 0x2c:u32{0,1,5000,599999,600000,600001,0x7fffffff}}, ptr[0x80]:bytes) -> i32")
