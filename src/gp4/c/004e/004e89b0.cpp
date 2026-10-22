#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004e89b0 { uint8_t Unknown00[0x3c]; uint32_t Flags3c; };
#pragma pack(pop)

// Restores the car slot selected by (mode - 1), refreshes the table pointer at 0x00a6987c
// (0x0051f9d4 preserves all registers), publishes it in 0x00a95db0 and sets bit
// (0x00a69878 & 31) in its flags dword at +0x3c.
// FUNCTION: GP4 0x004e89b0
int __cdecl FUN_004e89b0() {
    gp4::stdcall_<0x00510abf, void>(GP4_GLOBAL(int32_t, 0x00645950) - 1);
    const uint32_t bit = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::Regs c = {};
    gp4::call_regs(0x0051f9d4, c);
    Unknown_004e89b0* table = GP4_GLOBAL(Unknown_004e89b0*, 0x00a6987c);
    GP4_GLOBAL(Unknown_004e89b0*, 0x00a95db0) = table;
    table->Flags3c |= 1u << (bit & 31);
    return (int)table;
}
GP4_IMPL(0x004e89b0, FUN_004e89b0, "cdecl() -> i32; globals=fuzz:i32+layout=save_00510309+carbg_0066fef0+slotcar_00510abf+view_00a4df50+player_004e89b0+flags_0062ffa8")
