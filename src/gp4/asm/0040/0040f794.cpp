#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte-wide TEST / AND (CF = OF = AF = 0; PF, ZF, SF from the result).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0040f794
// Re-seeds the car record in esi: the word +0xe is reduced by the Q24 products of the dword +0x28c
// with 0x00603400 and with 0x00603854 and copied to +0x18; several state bits are cleared
// (+0xe5 bit 0, +0x71 bit 6, +0xcc bit 7, +0xe7 bit 0, +0xce bit 6; +0x1f7 = 0; +0xce bit 4 unless
// +0x163 bit 7) and 0x0042785a runs. Unless +0x8d bit 2 or +0x163 bit 7 is set, more bits are then
// cleared (+0xcc bit 4, +0x203 bit 1, +0xe5 bit 4, +0xff bit 3, +0x71 bit 0). eax / edx keep the
// last imul / shrd results (the word +0xe replaces ax); the flags are those of the last test / and.
void __cdecl FUN_0040f794(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint16_t* const heading = reinterpret_cast<uint16_t*>(car + 0xe);

    const int32_t factor = *reinterpret_cast<int32_t*>(car + 0x28c);

    int64_t product = static_cast<int64_t>(factor) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603400));
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 24);
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    *heading = static_cast<uint16_t>(*heading - static_cast<uint16_t>(eax));

    product = static_cast<int64_t>(factor) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 24);
    edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    *heading = static_cast<uint16_t>(*heading - static_cast<uint16_t>(eax));

    eax = (eax & 0xffff0000u) | *heading;
    *reinterpret_cast<uint16_t*>(car + 0x18) = *heading;

    car[0xe5] &= 0xfeu;
    car[0x71] &= 0xbfu;
    car[0xcc] &= 0x7fu;
    car[0xe7] &= 0xfeu;
    car[0xce] &= 0xbfu;
    car[0x1f7] = 0;
    if ((car[0x163] & 0x80u) == 0u) {
        car[0xce] &= 0xefu;
    }

    gp4::Regs sub = *r;
    sub.st_in = 0;
    sub.st_out = 0;
    sub.eax = eax;
    sub.edx = edx;
    gp4::call_regs(0x0042785a, sub);

    uint32_t flags = logicFlags8(car[0x8d] & 4u);
    if ((car[0x8d] & 4u) == 0u) {
        flags = logicFlags8(car[0x163] & 0x80u);
        if ((car[0x163] & 0x80u) == 0u) {
            car[0xcc] &= 0xefu;
            car[0x203] &= 0xfdu;
            car[0xe5] &= 0xefu;
            car[0xff] &= 0xf7u;
            car[0x71] &= 0xfeu;
            flags = logicFlags8(car[0x71]);
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f794, FUN_0040f794, "regs(edi:ptr[0x1c4]:bytes, esi:ptr[0x420]:layout=rec_004275d1+rec_00409416) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800")
