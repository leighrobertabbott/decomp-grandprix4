#include <gp4/gp4.h>

// FUNCTION: GP4 0x00430d07
void __cdecl FUN_00430d07(gp4::Regs* r) {
    // Clear two groups of counters (one every 8 bytes) ...
    int32_t* groupA = GP4_ARRAY(int32_t, 0x006041a4);
    groupA[-4] = 0;
    groupA[-2] = 0;
    groupA[0] = 0;
    groupA[2] = 0;
    groupA[4] = 0;
    groupA[6] = 0;
    groupA[8] = 0;
    GP4_GLOBAL(int32_t, 0x006043d8) = 0;
    GP4_GLOBAL(int32_t, 0x006043dc) = 0;

    int32_t* groupB = GP4_ARRAY(int32_t, 0x00604224);
    groupB[-4] = 0;
    groupB[-2] = 0;
    groupB[0] = 0;
    groupB[2] = 0;
    groupB[4] = 0;
    groupB[6] = 0;
    groupB[8] = 0;
    GP4_GLOBAL(int32_t, 0x00604274) = 0;
    GP4_GLOBAL(int32_t, 0x00604278) = 0;
    GP4_GLOBAL(int32_t, 0x006078dc) = 0;
    GP4_GLOBAL(int32_t, 0x006078e0) = 0;
    GP4_GLOBAL(int32_t, 0x00607960) = 0;
    GP4_GLOBAL(int32_t, 0x00607964) = 0;

    // ... then clear one word in each of the 22 records of 0x41c bytes.
    uint8_t* record = GP4_ARRAY(uint8_t, 0x0066fef0);
    for (int i = 0; i < 22; ++i) {
        GP4_FIELD(uint16_t, record, 0x23e) = 0;
        record += 0x41c;
    }

    // Final registers: the last "add esi, 0x41c" (0x67553c + 0x41c) leaves only AF set.
    r->ebx = 0x00604224u;
    r->ecx = 0;
    r->esi = reinterpret_cast<uintptr_t>(record);
    r->eflags = (r->eflags & ~0x8d5u) | 0x10u;
}
GP4_IMPL(0x00430d07, FUN_00430d07, "regs() -> (ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
