#include <gp4/gp4.h>

// pushal / popal wrapper: records one entry of the 16-byte table at 0x0062fc28, entry number [0x0062fc0c]
// (also copied to 0x0062fc14), from the flag record at ebx and the object at edi, then marks that entry
// number in the bit sets through 0x00434063. Only the flags of that call survive the popal.
//   +0  low byte of [0x0062fc10]            +1  byte [ebx + 1]
//   +2  0xff when word [ebx + 2] < 0        +3  0 when bit 7 of byte [ebx] is set, else 0xff
//   +4/+8  eax / edx returned by 0x00404796 (rotation seeded with ((word [edi + 0x60 or 0x62] + 0x50) *
//          word [ebx + 2]) >> 8)            +0xc  the object pointer edi
// FUNCTION: GP4 0x00441c0c
void __cdecl FUN_00441c0c(gp4::Regs* r) {
    uint8_t* flagRecord = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const uint32_t number = GP4_GLOBAL(uint32_t, 0x0062fc0c);
    GP4_GLOBAL(uint32_t, 0x0062fc14) = number;
    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(number * 0x10u + 0x0062fc28u));

    GP4_FIELD(uint32_t, entry, 0xc) = r->edi;
    entry[3] = (flagRecord[0] & 0x80u) != 0u ? 0u : 0xffu;

    const int32_t turn = GP4_FIELD(int16_t, flagRecord, 2);
    uint32_t span;
    if (turn < 0) {
        entry[2] = 0xff;
        span = GP4_FIELD(uint16_t, object, 0x60);
    } else {
        span = GP4_FIELD(uint16_t, object, 0x62);
        entry[2] = 0;
    }
    span += 0x50u;
    const int64_t product = static_cast<int64_t>(turn) * static_cast<int32_t>(span);

    gp4::Regs rot = *r;
    rot.st_in = 0;
    rot.st_out = 0;
    rot.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 8);
    rot.edx = 0;
    gp4::call_regs(0x00404796, rot);

    GP4_FIELD(uint32_t, entry, 4) = rot.eax;
    GP4_FIELD(uint32_t, entry, 8) = rot.edx;
    entry[1] = flagRecord[1];
    entry[0] = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x0062fc10));

    gp4::Regs mark = *r;
    mark.st_in = 0;
    mark.st_out = 0;
    mark.eax = 0;
    mark.edx = GP4_GLOBAL(uint32_t, 0x0062fc14);
    gp4::call_regs(0x00434063, mark);

    r->eflags = (r->eflags & ~0x8d5u) | (mark.eflags & 0x8d5u);
}
GP4_IMPL(0x00441c0c, FUN_00441c0c, "regs(ebx:ptr[0x40]:bytes, edi:ptr[0x6a]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00441c0c")
