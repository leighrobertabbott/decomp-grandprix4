#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00553d5f
// Splits the three fixed-point dwords 0x870640 / 0x870644 / 0x870648 into the record in edi: each
// raw value is kept at +0xf0 / +0xf4 / +0xf8; the first and third also give a 3-bit field (bits
// 8..10) packed into the byte at +0x21 (low nibble from the first, high nibble from the third) and
// a word (value >> 11) at +4 / +8; the second gives the word (value >> 8) at +6. ebx is saved and
// restored; eax ends as the third value >> 11 and the flags are those of that final `sar eax, 3`
// (CF = last bit shifted out, OF = AF = 0, the rest follow the result).
void __cdecl FUN_00553d5f(gp4::Regs* r) {
    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    int32_t value = GP4_GLOBAL(int32_t, 0x00870640);
    GP4_FIELD(int32_t, record, 0xf0) = value;
    value >>= 8;
    GP4_FIELD(uint8_t, record, 0x21) = static_cast<uint8_t>(value & 7);
    value >>= 3;
    GP4_FIELD(int16_t, record, 0x04) = static_cast<int16_t>(value);

    value = GP4_GLOBAL(int32_t, 0x00870644);
    GP4_FIELD(int32_t, record, 0xf4) = value;
    value >>= 8;
    GP4_FIELD(int16_t, record, 0x06) = static_cast<int16_t>(value);

    value = GP4_GLOBAL(int32_t, 0x00870648);
    GP4_FIELD(int32_t, record, 0xf8) = value;
    value >>= 8;
    const uint32_t nibble = (static_cast<uint32_t>(value) & 7u) << 4;
    GP4_FIELD(uint8_t, record, 0x21) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x21) | nibble);
    const uint32_t carry = (static_cast<uint32_t>(value) >> 2) & 1u;
    value >>= 3;
    GP4_FIELD(int16_t, record, 0x08) = static_cast<int16_t>(value);

    r->eax = static_cast<uint32_t>(value);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(static_cast<uint32_t>(value)) | (carry != 0u ? gp4::flags::CF : 0u));
}
GP4_IMPL(0x00553d5f, FUN_00553d5f, "regs(edi:ptr[0x100]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
