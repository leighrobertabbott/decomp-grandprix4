#include <gp4/gp4.h>

struct Device_005aafc0;
typedef int (__fastcall *Device_005aafc0_Slot34)(Device_005aafc0* self, void* edx, uint32_t first, int32_t second);

#pragma pack(push, 1)
struct Device_005aafc0 {
    uint32_t* Vtable00;
    uint8_t   Unknown04[0xfc];
};
struct Unknown_005aafc0 {
    uint8_t          Unknown00[0x1c];
    Device_005aafc0* Device1c;
    uint8_t          Unknown20[0x34];
    uint16_t         Word54;
    uint8_t          Flag56;
    uint8_t          Unknown57[0x15];
    int16_t          Value6c;
    uint8_t          Unknown6e[0x1a];
};
#pragma pack(pop)

// Stores the stack argument (floored at -1000 when its low word is below that) as a word at
// +0x6c. When the byte at +0x56 is 1 the same value goes on to virtual slot 0x34 (+0xd0) of the
// object at +0x1c together with the word at +0x54, otherwise returns 0. The word is loaded into
// the low half of the register that still holds `this`, so the first argument carries the upper
// 16 bits of the object's address (`mov eax, ecx ... mov ax, [eax+0x54]; push eax`).
// FUNCTION: GP4 0x005aafc0
int __fastcall FUN_005aafc0(Unknown_005aafc0* self, void* /*edx*/, int32_t value)
{
    if ((int16_t)value < -1000) {
        value = -1000;
    }
    uint8_t flag = self->Flag56;
    self->Value6c = (int16_t)value;
    if (flag == 1) {
        Device_005aafc0* dev = self->Device1c;
        uint32_t first = ((uint32_t)(uintptr_t)self & 0xffff0000u) | self->Word54;
        Device_005aafc0_Slot34 method = (Device_005aafc0_Slot34)dev->Vtable00[0xd0 / 4];
        return method(dev, 0, first, value);
    }
    return 0;
}
GP4_IMPL(0x005aafc0, FUN_005aafc0, "thiscall(ptr[0x100]:{0x1c:ptr[0x100]:{0:ptr[0xd8]:{0xd0:u32{=0x005bc6e0}}}, 0x54:u16, 0x56:u8{0,1}}, i32) -> i32")
