#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads the next command word w at esi (esi advances by 2, eax = w) and applies it to the object
// record at ebx: the table index i = w >> 4 is stored as a word at +0x1e, [0x00932440] becomes the
// largest index seen so far, and the 16-byte table entry i of the array at [0x0086850c] (its word
// 0) selects flag bytes of the record:
//   entry & 0x8000 -> +0x48 |= 0x40      entry & 0x100 -> +0x92 |= 4      +0x48 |= 0x80 (always)
//   no 0x800 -> +0x34 |= 0x80, then no 0x1000 -> +0x38 |= 0x80, then no 0x2000 -> +0x40 |= 0x80.
// pushal/popal restore every register except eax and esi; the flags are those of the last test
// (the 0x800, 0x1000 or 0x2000 one that stopped the cascade) or of the final `or` on +0x40.
// FUNCTION: GP4 0x00556dee
void __cdecl FUN_00556dee(gp4::Regs* r) {
    const uint32_t stream = r->esi;
    const uint16_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(stream));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* table = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0086850c)));

    const uint32_t index = static_cast<uint32_t>(word) >> 4;
    GP4_FIELD(uint16_t, record, 0x1e) = static_cast<uint16_t>(index);
    if (!(index < GP4_GLOBAL(uint32_t, 0x00932440))) {
        GP4_GLOBAL(uint32_t, 0x00932440) = index;
    }
    const uint32_t offset = index << 4;

    uint32_t arith;
    if ((GP4_FIELD(uint16_t, table, offset) & 0x8000u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x48) | 0x40u);
    }
    if ((GP4_FIELD(uint16_t, table, offset) & 0x100u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x92) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x92) | 4u);
    }
    GP4_FIELD(uint8_t, record, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x48) | 0x80u);

    arith = gp4::flags::logic16(GP4_FIELD(uint16_t, table, offset) & 0x800u);
    if ((GP4_FIELD(uint16_t, table, offset) & 0x800u) == 0u) {
        GP4_FIELD(uint8_t, record, 0x34) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x34) | 0x80u);
        arith = gp4::flags::logic16(GP4_FIELD(uint16_t, table, offset) & 0x1000u);
        if ((GP4_FIELD(uint16_t, table, offset) & 0x1000u) == 0u) {
            GP4_FIELD(uint8_t, record, 0x38) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x38) | 0x80u);
            arith = gp4::flags::logic16(GP4_FIELD(uint16_t, table, offset) & 0x2000u);
            if ((GP4_FIELD(uint16_t, table, offset) & 0x2000u) == 0u) {
                const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x40) | 0x80u);
                GP4_FIELD(uint8_t, record, 0x40) = value;
                arith = gp4::flags::logic8(value);
            }
        }
    }

    r->eax = word;
    r->esi = stream + 2u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00556dee, FUN_00556dee, "regs(ebx:ptr[0xa0]:bytes, esi:ptr[0x40]:u16[0..0x10f]) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_0086850c")
