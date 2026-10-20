#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_Stamp {
    uint32_t Low;
    uint32_t High;
};
#pragma pack(pop)

// Reads the performance counter (FUN_0054e733) into the qword 0x00830004 and returns the address of
// the first of the 22 0x1c-byte records at 0x00b1b8d8 that is in use (first dword nonzero and not
// equal to the dword 0x00b1b8d8 + 0x268) whose qword stamp (0x0083002b + 8*k, unaligned) is more
// than 0x4446390 counter ticks old; 0 when none. ebx, ecx, edx and esi are saved and restored.
// FUNCTION: GP4 0x0053fdcc
uint32_t __cdecl FUN_0053fdcc() {
    gp4::Regs c = {};
    gp4::call_regs(0x0054e733, c);
    GP4_GLOBAL(uint32_t, 0x00830004) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00830008) = c.edx;
    uint8_t* record = GP4_ARRAY(uint8_t, 0x00b1b8d8);
    const Unknown_Stamp* stamp = reinterpret_cast<const Unknown_Stamp*>(GP4_ARRAY(uint8_t, 0x0083002b));
    for (int left = 0x16; left != 0; --left) {
        const uint32_t first = GP4_FIELD(uint32_t, record, 0);
        if (first != 0u && first != GP4_GLOBAL(uint32_t, 0x00b1bb40)) {
            const uint32_t nowLow = GP4_GLOBAL(uint32_t, 0x00830004);
            const uint32_t nowHigh = GP4_GLOBAL(uint32_t, 0x00830008);
            const uint32_t low = nowLow - stamp->Low;
            const uint32_t high = nowHigh - stamp->High - (nowLow < stamp->Low ? 1u : 0u);
            if (static_cast<int32_t>(high) > 0 || (high == 0u && low > 0x4446390u)) {
                return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
            }
        }
        record += 0x1c;
        stamp += 1;
    }
    return 0u;
}
GP4_IMPL(0x0053fdcc, FUN_0053fdcc, "cdecl() -> u32; globals=fuzz:i32+layout=stamps_0053fdcc")
