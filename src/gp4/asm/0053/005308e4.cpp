#include <gp4/gp4.h>

// Three 0x3c-byte records at 0x827160 (0xb4 bytes in all); only the fields this routine writes are named.
struct Record827160 {
    int32_t Index00;
    uint8_t Unknown04;
    uint8_t Flags05;
    uint8_t Unknown06[0x36];
};
static_assert(sizeof(Record827160) == 0x3c, "record size");

// FUNCTION: GP4 0x005308e4
// Resets the 3-record table at 0x827160: clears all 0xb4 bytes (rep stosb, direction flag clear),
// then stamps record k with index k and byte +5 = 2. Leaves edi = ebx = 0x827214, ecx = 3,
// al = 0 and the flags of the final `cmp ecx, 3`.
void __cdecl FUN_005308e4(gp4::Regs* r) {
    uint8_t* bytes = GP4_ARRAY(uint8_t, 0x00827160);
    for (uint32_t i = 0; i < 0xb4u; ++i) {
        bytes[i] = 0;
    }

    Record827160* rec = GP4_ARRAY(Record827160, 0x00827160);
    uint32_t index = 0;
    do {
        rec->Flags05 = 2;
        rec->Index00 = (int32_t)index;
        ++rec;
        ++index;
    } while (index != 3u);

    r->eax = r->eax & ~0xffu;
    r->ebx = (uint32_t)rec;
    r->ecx = index;
    r->edi = 0x00827160u + 0xb4u;
    // cmp ecx, 3 with ecx == 3: result 0 -> ZF = 1, PF = 1, all other arithmetic flags clear.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x005308e4, FUN_005308e4, "regs(eax:u32) -> (eax, ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
