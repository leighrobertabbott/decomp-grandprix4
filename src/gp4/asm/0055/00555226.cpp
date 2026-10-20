#include <gp4/gp4.h>
#include <gp4/flags.h>

// Rescales the word at valueOff of the record by the word at baseOff: ratio = (value << 14) / max(base, 1)
// (unsigned 32-bit divide). A ratio above 0x4000 (signed compare) is replaced by base - value, which is
// negative whenever the ratio is that large (value > base), so the 0xffffffff replacement for a
// non-negative difference never happens. The low word of the result is stored back. Returns the flags
// of the last compare: `cmp ratio, 0x4000`, or the `sub base, value` when the ratio was above it.
static uint32_t RescaleWord(uint8_t* record, uint32_t baseOff, uint32_t valueOff) {
    uint32_t divisor = GP4_FIELD(uint16_t, record, baseOff);
    if (divisor == 0u) {
        divisor = 1u;
    }
    uint32_t result = (static_cast<uint32_t>(GP4_FIELD(uint16_t, record, valueOff)) << 14) / divisor;
    uint32_t arith = gp4::flags::sub32(result, 0x4000u);
    if (static_cast<int32_t>(result) > 0x4000) {
        const uint32_t base = GP4_FIELD(uint16_t, record, baseOff);
        const uint32_t value = GP4_FIELD(uint16_t, record, valueOff);
        result = base - value;
        arith = gp4::flags::sub32(base, value);
        if (static_cast<int32_t>(result) >= 0) {
            result = 0xffffffffu;
        }
    }
    GP4_FIELD(uint16_t, record, valueOff) = static_cast<uint16_t>(result);
    return arith;
}

// Ring-record fraction fix-up (pushal/popal wrapper): the words +0x7e and +0x80 are turned into 14-bit
// fractions of the limits +0x74 and +0x76 (0x4000 = the whole limit); a value beyond its limit is
// stored as limit - value (negative). All registers are restored; the flags are those of the last
// compare of the second word.
// FUNCTION: GP4 0x00555226
void __cdecl FUN_00555226(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    RescaleWord(record, 0x74u, 0x7eu);
    const uint32_t arith = RescaleWord(record, 0x76u, 0x80u);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00555226, FUN_00555226, "regs(edi:ptr[0x90]:u16{0,0,1,2,3,0x10,0x3fff,0x4000,0x4001,0x8000,0xffff}) -> (cf, pf, af, zf, sf, of)")
