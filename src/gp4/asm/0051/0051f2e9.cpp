#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
// The parts of the 0x146-byte record in edi that the routine reads (words of 0xffff mean "none").
struct Unknown_Lane {
    uint8_t Unknown00[0x7a];
    uint16_t Word7a;
    uint8_t Unknown7c[0x0c];
    uint16_t Word88;
    uint16_t Word8a;
    uint8_t Unknown8c[0x10];
    uint16_t Word9c;
    uint8_t Unknown9e;
    uint8_t Flags9f;
    uint8_t Unknowna0[0x98];
    uint16_t Word138;
    uint8_t Unknown13a[4];
    uint16_t Word13e;
};
#pragma pack(pop)

// Builds the availability bits in the byte at +0x9f from the record's words: bit 0 when +0x88 is set;
// bit 1 when +0x9c is set, or (unless 0xa53ad0 == 0 and 0x64594c == 1) when +0x138 is set; bit 2 the same
// with +0x8a and +0x13e; bit 3 when +0x7a is set. The flags are those of the last instruction: the
// closing `or byte [edi+0x9f], 8` when +0x7a is set, else `cmp word [edi+0x7a], 0xffff`.
// FUNCTION: GP4 0x0051f2e9
void __cdecl FUN_0051f2e9(gp4::Regs* r) {
    Unknown_Lane* lane = reinterpret_cast<Unknown_Lane*>(static_cast<uintptr_t>(r->edi));
    if (lane->Word88 != 0xffffu) {
        lane->Flags9f |= 1u;
    }
    if (lane->Word9c != 0xffffu ||
        ((GP4_GLOBAL(uint32_t, 0x00a53ad0) != 0u || GP4_GLOBAL(uint32_t, 0x0064594c) != 1u) &&
         lane->Word138 != 0xffffu)) {
        lane->Flags9f |= 2u;
    }
    if (lane->Word8a != 0xffffu ||
        ((GP4_GLOBAL(uint32_t, 0x00a53ad0) != 0u || GP4_GLOBAL(uint32_t, 0x0064594c) != 1u) &&
         lane->Word13e != 0xffffu)) {
        lane->Flags9f |= 4u;
    }
    uint32_t arith;
    const uint16_t last = lane->Word7a;
    if (last != 0xffffu) {
        lane->Flags9f |= 8u;
        arith = gp4::flags::logic8(lane->Flags9f);
    } else {
        arith = gp4::flags::sub16(last, 0xffffu);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051f2e9, FUN_0051f2e9, "regs(edi:ptr[0x146]:u16{0xffff,0xffff,0,1,2}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
