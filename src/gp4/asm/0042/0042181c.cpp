#include <gp4/gp4.h>

// Address of the dword at byte offset `offset` from `base` (kept out of line so the compiler does
// not fold base + offset into separate absolute globals).
static __declspec(noinline) uint32_t* slotAt(uint8_t* base, uint32_t offset) {
    return reinterpret_cast<uint32_t*>(base + offset);
}

// FUNCTION: GP4 0x0042181c
// Copies the dwords at esi+0x40c / esi+0x410 into dword +0x110 of the four 0x300-byte records at
// 0x006380d8 (records 0 and 1 get the first value, records 2 and 3 the second), then for each
// record stores +0xf4 = +0x140 + +0x120 and +0xf0 = +0x13c + +0x11c + +0x110.
// pushal/popal preserve every GPR; the flags are those of the last "cmp ebx, 0x638cd8" (equal).
void __cdecl FUN_0042181c(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* base = GP4_ARRAY(uint8_t, 0x006380d8);
    const uint32_t first = GP4_FIELD(uint32_t, source, 0x40c);
    const uint32_t second = GP4_FIELD(uint32_t, source, 0x410);

    *slotAt(base, 0x110) = first;
    *slotAt(base, 0x410) = first;
    *slotAt(base, 0x710) = second;
    *slotAt(base, 0xa10) = second;

    uint32_t walker = 0x006380d8u;
    do {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(walker));
        GP4_FIELD(uint32_t, record, 0xf4) = GP4_FIELD(uint32_t, record, 0x140) + GP4_FIELD(uint32_t, record, 0x120);
        GP4_FIELD(uint32_t, record, 0xf0) =
            GP4_FIELD(uint32_t, record, 0x13c) + GP4_FIELD(uint32_t, record, 0x11c) + GP4_FIELD(uint32_t, record, 0x110);
        walker += 0x300u;
    } while (walker != 0x00638cd8u);

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0042181c, FUN_0042181c, "regs(esi:ptr[0x418]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sums_0042181c")
