#include <gp4/gp4.h>

// 16-bit register halves go in and out of the callees; the upper halves keep the entry values.
static inline uint32_t withLow16(uint32_t entry, uint32_t low) {
    return (entry & 0xffff0000u) | (low & 0xffffu);
}

// FUNCTION: GP4 0x0040a115
// Cosine from a sine in 2.30 fixed point: [0x934dec] = sqrt(2^60 - [0x934de8]^2).
// The dword [0x934de8] is squared with the 16-bit multiplier 0x00401000 (64-bit product in
// bx:cx:dx:ax stored to [0x62ad50], [0x62ad52], [0x934dec], [0x934dee]); 0x1000_0000_0000_0000 minus the
// product is built in the words [0x62ad46]:[0x62ad44]:[0x934de2]:[0x934de0] with sub/sbb. When that
// difference is negative (sign word [0x62ad46]) the upper dword [0x62ad50] and the low dword [0x934de0]
// are cleared, so the root is 0. The difference then goes through the 64-bit integer square root
// 0x00409cf2 (result back in [0x934dec]). Both calls sit between pushal/popal; the only register change
// that survives is the pair of "mov ax,[0x62ad50] / mov dx,[0x62ad52]" loads between them, which leave
// ax = product bits 32..47 and dx = product bits 48..63 (upper halves unchanged). The flags are those
// 0x00409cf2 leaves.
void __cdecl FUN_0040a115(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00934de8);
    GP4_GLOBAL(uint32_t, 0x00934dec) = value;

    gp4::Regs square = *r;
    square.st_in = 0;
    square.st_out = 0;
    square.eax = withLow16(r->eax, GP4_GLOBAL(uint16_t, 0x00934dec));
    square.edx = withLow16(r->edx, GP4_GLOBAL(uint16_t, 0x00934dee));
    square.ebx = withLow16(r->ebx, GP4_GLOBAL(uint16_t, 0x00934dec));
    square.ecx = withLow16(r->ecx, GP4_GLOBAL(uint16_t, 0x00934dee));
    gp4::call_regs(0x00401000, square);
    GP4_GLOBAL(uint16_t, 0x0062ad50) = static_cast<uint16_t>(square.ebx);
    GP4_GLOBAL(uint16_t, 0x0062ad52) = static_cast<uint16_t>(square.ecx);
    GP4_GLOBAL(uint16_t, 0x00934dec) = static_cast<uint16_t>(square.eax);
    GP4_GLOBAL(uint16_t, 0x00934dee) = static_cast<uint16_t>(square.edx);

    const uint32_t highLow = GP4_GLOBAL(uint16_t, 0x0062ad50);
    const uint32_t highHigh = GP4_GLOBAL(uint16_t, 0x0062ad52);
    const uint64_t product = (static_cast<uint64_t>(highHigh) << 48)
        | (static_cast<uint64_t>(highLow) << 32)
        | (static_cast<uint64_t>(GP4_GLOBAL(uint16_t, 0x00934dee)) << 16)
        | static_cast<uint64_t>(GP4_GLOBAL(uint16_t, 0x00934dec));
    const uint64_t difference = 0x1000000000000000ull - product;

    GP4_GLOBAL(uint16_t, 0x0062ad44) = static_cast<uint16_t>(difference >> 32);
    GP4_GLOBAL(uint16_t, 0x0062ad46) = static_cast<uint16_t>(difference >> 48);
    GP4_GLOBAL(uint16_t, 0x00934de0) = static_cast<uint16_t>(difference);
    GP4_GLOBAL(uint16_t, 0x00934de2) = static_cast<uint16_t>(difference >> 16);
    GP4_GLOBAL(uint32_t, 0x0062ad50) = GP4_GLOBAL(uint32_t, 0x0062ad44);
    if (static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x0062ad46)) < 0) {
        GP4_GLOBAL(uint32_t, 0x0062ad50) = 0;
        GP4_GLOBAL(uint32_t, 0x00934de0) = 0;
    }
    GP4_GLOBAL(uint32_t, 0x00934dec) = GP4_GLOBAL(uint32_t, 0x00934de0);

    gp4::Regs root = *r;
    root.st_in = 0;
    root.st_out = 0;
    root.ebx = withLow16(r->ebx, GP4_GLOBAL(uint16_t, 0x0062ad50));
    root.ecx = withLow16(r->ecx, GP4_GLOBAL(uint16_t, 0x0062ad52));
    root.eax = withLow16(r->eax, GP4_GLOBAL(uint16_t, 0x00934dec));
    root.edx = withLow16(r->edx, GP4_GLOBAL(uint16_t, 0x00934dee));
    gp4::call_regs(0x00409cf2, root);
    GP4_GLOBAL(uint16_t, 0x00934dec) = static_cast<uint16_t>(root.eax);
    GP4_GLOBAL(uint16_t, 0x00934dee) = static_cast<uint16_t>(root.edx);

    r->eax = withLow16(r->eax, highLow);
    r->edx = withLow16(r->edx, highHigh);
    r->eflags = root.eflags;
}
GP4_IMPL(0x0040a115, FUN_0040a115, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
