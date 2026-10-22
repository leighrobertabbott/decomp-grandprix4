#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005adc80_Target { uint32_t* Vtable00; };
struct Unknown_005adc80 {
    uint8_t Unknown00[0x25c];
    Unknown_005adc80_Target** Table25c;
};
#pragma pack(pop)

typedef int (__stdcall *Unknown_005adc80_Method)(Unknown_005adc80_Target* self, int value);

// Looks up the object at index (a & 0xffff) of the table at +0x25c; a missing entry reports
// true, otherwise vtable slot 0x40 of the entry is called (stdcall, object then the sign
// extended 16-bit value) and its result is reduced to 0 or 1.
// FUNCTION: GP4 0x005adc80
int32_t __fastcall FUN_005adc80(Unknown_005adc80* self, void* /*edx*/, uint32_t index, int16_t value) {
    Unknown_005adc80_Target* target = self->Table25c[index & 0xffff];
    if (target == 0) {
        return 1;
    }
    Unknown_005adc80_Method method = (Unknown_005adc80_Method)target->Vtable00[0x40 / 4];
    int32_t result = method(target, (int)value);
    return result != 0;
}
GP4_IMPL(0x005adc80, FUN_005adc80, "thiscall(ptr[0x264]:{0x25c:ptr[0x40]:{0*16/4:nptr[8]:{0:ptr[0x44]:{0x40:u32{=0x005a82a0}}}}}, u32{=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0x10003,0x2000a,0xffff0007}, i32) -> i32")
