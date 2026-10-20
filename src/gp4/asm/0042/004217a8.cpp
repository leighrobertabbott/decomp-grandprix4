#include <gp4/gp4.h>

// One 0x300-byte record of the four at 0x006380d8; only the fields this routine touches are named.
struct Unknown_004217a8_Record {
    uint8_t Unknown00[0x9c];
    int32_t Unknown9C;
    uint8_t UnknownA0[0xf0 - 0xa0];
    int32_t UnknownF0;
    int32_t UnknownF4;
    int32_t UnknownF8;
    int32_t UnknownFC;
    int32_t Unknown100;
    int32_t Unknown104;
};

// For each of the four records at 0x006380d8 (stride 0x300), with q = [0x00602bc0]:
//   t   = (+0xf0 * q) >> 6                  (64-bit product, low half of the shifted value)
//   +0xf8  = 3 * (+0xf4 << 10) + t
//   +0xfc  = ((3 * +0x9c) << 22) / q        (64-bit idiv)
//   +0x104 = +0xfc + +0xf8
//   +0x100 = t + +0xfc
// pushal/popal keep every GPR; the flags are those of the last "cmp ebx, 0x638cd8" (equal).
// FUNCTION: GP4 0x004217a8
void __cdecl FUN_004217a8(gp4::Regs* r) {
    uintptr_t walker = 0x006380d8u;
    do {
        Unknown_004217a8_Record* record = reinterpret_cast<Unknown_004217a8_Record*>(walker);

        const int64_t product = static_cast<int64_t>(record->UnknownF0) * GP4_GLOBAL(int32_t, 0x00602bc0);
        uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 6);

        const uint32_t base = static_cast<uint32_t>(record->UnknownF4) << 10;
        record->UnknownF8 = static_cast<int32_t>(base + base * 2u + scaled);

        const int32_t numerator = static_cast<int32_t>(static_cast<uint32_t>(record->Unknown9C) * 3u);
        int32_t remainder;
        const int32_t quotient = gp4::idiv64(static_cast<int64_t>(numerator) * 0x400000, GP4_GLOBAL(int32_t, 0x00602bc0), &remainder);
        record->UnknownFC = quotient;
        record->Unknown104 = static_cast<int32_t>(static_cast<uint32_t>(quotient) + static_cast<uint32_t>(record->UnknownF8));
        scaled += static_cast<uint32_t>(record->UnknownFC);
        record->Unknown100 = static_cast<int32_t>(scaled);

        walker += 0x300u;
    } while (walker != 0x00638cd8u);

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004217a8, FUN_004217a8, "regs(esi:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_004217a8")
