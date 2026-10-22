#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046b310_Arg {
    uint8_t Unknown00[0xc];
    uint32_t Flags0c;
};
struct Unknown_0046b310_Rec {
    uint8_t Unknown00[0x80];
    int32_t Start80;
    int32_t Length84;
};
struct Unknown_0046b310_Slot {
    Unknown_0046b310_Rec* Rec00;
    uint32_t Unknown04;
    uint32_t Unknown08;
};
#pragma pack(pop)

// With the flag at 0x00a05c28 clear, returns bit 0x10 of the dword at +0xc of the argument. Otherwise
// takes the record of the 12-byte slot array at 0x00a0370c selected by the index at 0x00a05c2c and
// returns whether the CRT position (ftell, 0x005dcc00) of the file in 0x00a03a18 has reached
// start (+0x80) + length (+0x84).
// FUNCTION: GP4 0x0046b310
uint32_t __stdcall FUN_0046b310(Unknown_0046b310_Arg* arg) {
    if (GP4_GLOBAL(int32_t, 0x00a05c28) == 0) {
        return arg->Flags0c & 0x10;
    }
    int32_t index = GP4_GLOBAL(int32_t, 0x00a05c2c);
    int32_t file = GP4_GLOBAL(int32_t, 0x00a03a18);
    Unknown_0046b310_Rec* rec = GP4_ARRAY(Unknown_0046b310_Slot, 0x00a0370c)[index].Rec00;
    int32_t position = gp4::cdecl_<0x005dcc00, int>(file);
    return position >= rec->Length84 + rec->Start80 ? 1u : 0u;
}
GP4_IMPL(0x0046b310, FUN_0046b310, "stdcall(ptr[0x20]:{0xc:u32[0..0x1f]}) -> u32; gfields={0x00a05c28:u32{=0,1}, 0x00a05c2c:u32{=0,1,2,3}, 0x00a0370c*4/12:ptr[0x90]:bytes, 0x00a03a18:ptr[0x40]:bytes}; emu=skip:win32 EnterCriticalSection")
