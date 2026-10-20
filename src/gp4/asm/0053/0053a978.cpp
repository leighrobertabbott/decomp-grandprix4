#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu slider value mapping. The value in eax is clamped to the range spanned by the dwords +0x22
// and +0x26 (either order), shifted by the dword +0x2a, scaled by the word difference (+8) - (+4),
// the half range (+0x2e - +0x2a) / 2 is added (64-bit), and the sum is divided by the range
// +0x2e - +0x2a (0 when that range is empty). The quotient moves the word +4 into ebp. Then the
// routine stores (bit 5 of the byte +0x1f ? 0x3df1 : 0x1a8d) through 0x00531af4 and calls the empty
// routine 0x00531b2e. ebx, eax and ecx come back as the zero-extended words +4, +6 and +0xa; edx is
// the division remainder (the high dword of the sum when the range is empty); the flags are those
// the store routine leaves.
// FUNCTION: GP4 0x0053a978
void __cdecl FUN_0053a978(gp4::Regs* r) {
    const uint32_t record = r->esi;
    int32_t value = static_cast<int32_t>(r->eax);
    const int32_t first = GP4_FIELD(int32_t, record, 0x22);
    const int32_t second = GP4_FIELD(int32_t, record, 0x26);
    if (second < first) {
        if (value > first) value = first;
        if (value < second) value = second;
    } else {
        if (value < first) value = first;
        if (value > second) value = second;
    }

    const uint32_t origin = GP4_FIELD(uint32_t, record, 0x2a);
    const int32_t offset = static_cast<int32_t>(static_cast<uint32_t>(value) - origin);
    const int32_t word_delta = static_cast<int32_t>(GP4_FIELD(uint16_t, record, 8)) -
                               static_cast<int32_t>(GP4_FIELD(uint16_t, record, 4));
    const int64_t product = static_cast<int64_t>(offset) * static_cast<int64_t>(word_delta);

    const int32_t range = static_cast<int32_t>(GP4_FIELD(uint32_t, record, 0x2e) - origin);
    const int64_t sum = product + (static_cast<int64_t>(range) >> 1);

    int32_t quotient = 0;
    int32_t remainder = static_cast<int32_t>(static_cast<uint64_t>(sum) >> 32);
    if (range != 0) {
        quotient = gp4::idiv64(sum, range, &remainder);
    }

    const uint32_t word4 = GP4_FIELD(uint16_t, record, 4);
    const uint32_t word6 = GP4_FIELD(uint16_t, record, 6);
    const uint32_t wordA = GP4_FIELD(uint16_t, record, 0xa);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (GP4_FIELD(uint8_t, record, 0x1f) & 0x20u) == 0u ? 0x1a8du : 0x3df1u;
    c.ebx = word4;
    c.ecx = wordA;
    c.edx = static_cast<uint32_t>(remainder);
    c.ebp = word4 + static_cast<uint32_t>(quotient);
    gp4::call_regs(0x00531af4, c);
    gp4::cdecl_<0x00531b2e, void>();

    r->eax = word6;
    r->ebx = word4;
    r->ecx = wordA;
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = word4 + static_cast<uint32_t>(quotient);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053a978, FUN_0053a978, "regs(eax:i32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:u16{0,1,2,3}) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
