#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x00557c78 for a record: when the mode byte 0x0086857c is 2, the eight words at esi are
// copied into the record at edi: +0x144, +0x146, byte +0x136, +0x148, +0x14a, +0x14c, byte +0x137,
// +0x14e (words 2 and 6 are stored as bytes). ax ends as the last word read (upper half kept). esi
// advances by 0x10 in both cases. The flags are those of that `add esi, 0x10` when the mode is
// not 2, else those of the `cmp byte [0x0086857c], 2` (equal).
// FUNCTION: GP4 0x00557cb8
void __cdecl FUN_00557cb8(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0086857c);
    if (mode != 2u) {
        r->esi = esi_in + 0x10u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 0x10u));
        return;
    }
    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    GP4_FIELD(uint16_t, record, 0x144) = words[0];
    GP4_FIELD(uint16_t, record, 0x146) = words[1];
    GP4_FIELD(uint8_t, record, 0x136) = static_cast<uint8_t>(words[2]);
    GP4_FIELD(uint16_t, record, 0x148) = words[3];
    GP4_FIELD(uint16_t, record, 0x14a) = words[4];
    GP4_FIELD(uint16_t, record, 0x14c) = words[5];
    GP4_FIELD(uint8_t, record, 0x137) = static_cast<uint8_t>(words[6]);
    const uint16_t last = words[7];
    GP4_FIELD(uint16_t, record, 0x14e) = last;
    r->eax = (r->eax & 0xffff0000u) | last;
    r->esi = esi_in + 0x10u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 2u));
}
GP4_IMPL(0x00557cb8, FUN_00557cb8, "regs(eax:u32, esi:ptr[0x18]:bytes, edi:ptr[0x160]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
