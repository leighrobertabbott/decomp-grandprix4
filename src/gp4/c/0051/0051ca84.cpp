#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the resource slot offset (word 0x009e1b20[0x322], 0xffff = none). When present, the
// record at 0x006975ec + offset supplies a source pointer (+0x0c) and a byte count (word +0x10);
// that many bytes are copied from the pointer to 0x00968f5c, and the same again for the next
// three 0x100-byte blocks of source and destination.
// pushal/popal wrapper: every GPR is restored; the flags are those of the last flag writer:
// "cmp bx, -1" when no slot is selected, otherwise the closing `dec ebp` (CF kept from the
// last `add esi, 0x100`).
static __declspec(noinline) uint32_t SlotOffset(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

// FUNCTION: GP4 0x0051ca84
void __cdecl FUN_0051ca84(gp4::Regs* r) {
    const uint32_t offset = SlotOffset(0x322u);
    uint32_t arith;
    if ((uint16_t)offset == 0xffffu) {
        arith = gp4::flags::sub16(offset, 0xffffu);
    } else {
        const uint32_t record = offset + 0x006975ecu;
        uint32_t source = GP4_FIELD(uint32_t, record, 0x0c);
        uint8_t* dest = GP4_ARRAY(uint8_t, 0x00968f5c);
        uint32_t carry = 0u;
        for (uint32_t block = 0u; block < 4u; ++block) {
            const uint32_t length = GP4_FIELD(uint16_t, record, 0x10);
            for (uint32_t i = 0u; i < length; ++i) {
                dest[block * 0x100u + i] = GP4_FIELD(uint8_t, source, i);
            }
            carry = gp4::flags::add32(source, 0x100u) & gp4::flags::CF;
            source += 0x100u;
        }
        arith = gp4::flags::dec32(1u, carry);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051ca84, FUN_0051ca84, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
