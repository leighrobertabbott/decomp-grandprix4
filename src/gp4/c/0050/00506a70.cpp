#include <gp4/gp4.h>

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);

#pragma pack(push, 1)
// DIEFFECTINFOA as handed to the EnumEffects callback.
struct EffectInfo_00506a70 {
    uint32_t Size00;
    uint32_t Guid04[4];
    uint8_t Unknown14[0xc];
    char Name20[260];
};
#pragma pack(pop)

// EnumEffects callback: logs the effect name and, when it reads "Raw Force", records its GUID
// in the four dwords at 0x00ae6958.
// FUNCTION: GP4 0x00506a70
int __stdcall DInput_EnumEffectsCallback(EffectInfo_00506a70* info, void* /*pvRef*/) {
    GP4_IAT(OutputDebugStringA_t, 0x005f0290)(info->Name20);
    GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0063e31c));
    const char* want = GP4_ARRAY(const char, 0x0066d094);
    bool same = true;
    for (int i = 0; i < 10; i++) {
        if (info->Name20[i] != want[i]) {
            same = false;
            break;
        }
    }
    if (same) {
        GP4_GLOBAL(uint32_t, 0x00ae6958) = info->Guid04[0];
        GP4_GLOBAL(uint32_t, 0x00ae695c) = info->Guid04[1];
        GP4_GLOBAL(uint32_t, 0x00ae6960) = info->Guid04[2];
        GP4_GLOBAL(uint32_t, 0x00ae6964) = info->Guid04[3];
    }
    return 1;
}
GP4_IMPL(0x00506a70, DInput_EnumEffectsCallback,
         "stdcall(ptr[0x130]:{4:u32, 8:u32, 0xc:u32, 0x10:u32, 0x20:u32{0x20776152,0x20776152,0x41414141,0}, 0x24:u32{0x63726f46,0x63726f46,0x42424242}, 0x28:u16{0x65,0x65,0x66}}, i32) -> i32")
