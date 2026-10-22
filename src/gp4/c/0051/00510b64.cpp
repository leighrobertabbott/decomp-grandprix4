#include <gp4/gp4.h>

// Looks up car record `index` (clamped to 0..21 by 0x005106f6, which returns it in esi) and latches
// two display values in 0xa69878 / 0xa6987c. In mode 0 ([0x7c2e75] == 0) the table row is the
// record's number byte +0x73 minus one; otherwise it is the byte of table 0x7c2b24 selected by
// ((+0x7c & 0x3f) - 1), merged into the low byte of that (possibly 0xffffffff) index. The row
// indexes the word table 0x7c2acc (4 bytes per row): 0xa69878 = word - 0x334; 0xa6987c = record
// byte +0x7f. pushal/popal keeps every register.
// FUNCTION: GP4 0x00510b64
void __stdcall FUN_00510b64(int index) {
    gp4::Regs c = {};
    c.ecx = static_cast<uint32_t>(index);
    gp4::call_regs(0x005106f6, c);
    const uint32_t record = c.esi;

    uint32_t row;
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0) {
        row = GP4_FIELD(uint8_t, record, 0x73) - 1u;
    } else {
        const uint32_t slot = (GP4_FIELD(uint8_t, record, 0x7c) & 0x3fu) - 1u;
        row = (slot & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x007c2b24)[slot];
    }
    row <<= 1;
    const uint32_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(0x007c2accu + row * 2u));
    GP4_GLOBAL(uint32_t, 0x00a69878) = word - 0x334u;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = GP4_FIELD(uint8_t, record, 0x7f);
}
GP4_IMPL(0x00510b64, FUN_00510b64, "stdcall(i32[-8..40]) -> void; globals=fuzz:i32+layout=carbg_0066fef0")
