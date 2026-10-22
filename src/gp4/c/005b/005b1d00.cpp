#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005b1d00 {
    const void* Vptr00;
    uint16_t    Value04;
    uint8_t     Unknown06[0x24];
    uint16_t    Value2a;
    uint32_t    Value2c;
    uint8_t     Flag30;
    uint8_t     Flag31;
    uint16_t    Value32;
    uint32_t    Value34;
    uint32_t    Value38;
    uint32_t    Value3c;
};
#pragma pack(pop)

// Constructor-like routine: stores the 16-bit stack argument at +4 and the vtable pointer
// 0x005f9e8c at +0, clears two flag bytes and initialises a block of 10000 / 0 defaults.
// Returns the object.
// FUNCTION: GP4 0x005b1d00
Object_005b1d00* __fastcall FUN_005b1d00(Object_005b1d00* self, void* /*edx*/, uint16_t value)
{
    self->Value04 = value;
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f9e8c);
    self->Flag30 = 0;
    self->Flag31 = 0;
    self->Value2a = 10000;
    self->Value2c = 0;
    self->Value32 = 10000;
    self->Value34 = 0;
    self->Value38 = 10000;
    self->Value3c = 0;
    return self;
}
GP4_IMPL(0x005b1d00, FUN_005b1d00, "thiscall(ptr[0x48]:bytes, u32) -> i32")
