#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scales the vertex triples of two records of the pointer table 0x007c8d70 (entries 3 and -5, i.e.
// the dwords 0x007c8d7c and 0x007c8d5c): each record gets bit 0x1000 set in its word +0x2c and then
// every 16-byte vertex between the pointers +0x10 and +0x14 has its dwords +0, +4, +8 multiplied (low
// 32 bits, signed imul) by 0x00811a38, 0x00811a3c and 0x00811a38. pushal/popal keeps every register;
// the flags are those of the final `cmp edi, ebp` of the second loop (edi == ebp).
static __declspec(noinline) uint32_t TableEntry(int32_t index) {
    return GP4_ARRAY(uint32_t, 0x007c8d70)[index];
}

static void ScaleRecord(uint32_t record) {
    GP4_FIELD(uint16_t, record, 0x2c) |= 0x1000u;
    uint32_t cursor = GP4_FIELD(uint32_t, record, 0x10);
    const uint32_t end = GP4_FIELD(uint32_t, record, 0x14);
    do {
        GP4_FIELD(uint32_t, cursor, 0) = GP4_FIELD(uint32_t, cursor, 0) * GP4_GLOBAL(uint32_t, 0x00811a38);
        GP4_FIELD(uint32_t, cursor, 4) = GP4_FIELD(uint32_t, cursor, 4) * GP4_GLOBAL(uint32_t, 0x00811a3c);
        GP4_FIELD(uint32_t, cursor, 8) = GP4_FIELD(uint32_t, cursor, 8) * GP4_GLOBAL(uint32_t, 0x00811a38);
        cursor += 0x10u;
    } while (cursor != end);
}

// FUNCTION: GP4 0x005279d8
void __cdecl FUN_005279d8(gp4::Regs* r) {
    ScaleRecord(TableEntry(3));
    ScaleRecord(TableEntry(-5));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x005279d8, FUN_005279d8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
