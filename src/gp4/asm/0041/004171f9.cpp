#include <gp4/gp4.h>

struct Unknown_004171f9 {
    uint8_t Unknown00[0xe5];
    uint8_t FlagsE5;
    uint8_t UnknownE6[0x20e];
    int32_t Unknown2F4;
    int32_t Unknown2F8;
};

// Flags of a byte logic operation (AND/OR/TEST): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint8_t value) {
    uint32_t parity = value;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Byte at block + offset; kept out of line so the block base stays 0x006380d8 (the original
// addresses both bytes relative to ebx).
static __declspec(noinline) uint8_t blockByte(const uint8_t* block, uint32_t offset) {
    return block[offset];
}

// Sets flag bits +0xe5 of the record at esi from bit 7 of a byte of DAT_006380d8's block chosen by
// comparing the dwords +0x2f4 / +0x2f8: bit 7 set forces 0x80|0x20, otherwise 0x20 is cleared and
// bit 7 follows bit 6. eax's low byte receives the chosen byte, edx the dword +0x2f4, ebx the block.
// The flags are those of the last logic operation on the byte at +0xe5.
// FUNCTION: GP4 0x004171f9
void __cdecl FUN_004171f9(gp4::Regs* r) {
    Unknown_004171f9* self = reinterpret_cast<Unknown_004171f9*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* block = GP4_ARRAY(uint8_t, 0x006380d8);

    const int32_t first = self->Unknown2F4;
    uint8_t selector = blockByte(block, 0x62e);
    if (first < self->Unknown2F8) {
        selector = blockByte(block, 0x92e);
    }

    uint8_t value;
    if ((selector & 0x80u) != 0u) {
        value = static_cast<uint8_t>(self->FlagsE5 | 0x80u | 0x20u);
    } else {
        value = static_cast<uint8_t>(self->FlagsE5 & 0xdfu);
        if ((value & 0x40u) != 0u) {
            value = static_cast<uint8_t>(value | 0x80u);
        } else {
            value = static_cast<uint8_t>(value & 0x7fu);
        }
    }
    self->FlagsE5 = value;

    r->ebx = 0x006380d8;
    r->edx = static_cast<uint32_t>(first);
    r->eax = (r->eax & 0xffffff00u) | selector;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(value);
}
GP4_IMPL(0x004171f9, FUN_004171f9, "regs(esi:ptr[0x300]:bytes, eax:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=bits_004171f9")
