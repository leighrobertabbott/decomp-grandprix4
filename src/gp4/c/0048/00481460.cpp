#include <gp4/gp4.h>

// Viewport flag records of 0x218 bytes at 0x00a4b430. Clears the six flag bytes of the record
// selected by the dword 0x00a4df50, then, if the byte 0x007c2e6c is set, picks the record numbered
// by the dword 0x00645950 (record 0 otherwise) and sets its flag byte +4 to 1. The original leaves
// eax as that record number times 0x43 (the record stride in 8-byte units), which the caller stores
// at 0x00852d38.
// FUNCTION: GP4 0x00481460
int __cdecl FUN_00481460() {
    int view = GP4_GLOBAL(int, 0x00a4df50);
    unsigned int player = GP4_GLOBAL(unsigned int, 0x00645950);
    int cur = view * 0x43 * 8;
    GP4_ARRAY(uint8_t, 0x00a4b430)[cur] = 0;
    GP4_ARRAY(uint8_t, 0x00a4b431)[cur] = 0;
    GP4_ARRAY(uint8_t, 0x00a4b432)[cur] = 0;
    GP4_ARRAY(uint8_t, 0x00a4b433)[cur] = 0;
    GP4_ARRAY(uint8_t, 0x00a4b434)[cur] = 0;
    GP4_ARRAY(uint8_t, 0x00a4b435)[cur] = 0;
    int record = (int)((GP4_GLOBAL(uint8_t, 0x007c2e6c) != 0 ? 0xffffffffu : 0u) & player) * 0x43;
    GP4_ARRAY(uint8_t, 0x00a4b434)[record * 8] = 1;
    return record;
}
GP4_IMPL(0x00481460, FUN_00481460, "cdecl() -> i32; globals=fuzz:i32+layout=view_00a4df50")
