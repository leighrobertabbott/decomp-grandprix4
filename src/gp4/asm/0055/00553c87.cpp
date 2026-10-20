#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x00553bd1 for the second set of fields of the 0x160-byte track record (edi): the fraction is
// min(word[+0x14a] << 6, dword[+0x10c]) << 14 / dword[+0x10c] (zero divisor -> 1); X goes from
// a = (i16[+0xd6] << 8) + dword[+0xf0] to dword[+0x120] (0x934df0), Y from (i16[+0xde] << 8) + dword[+0xf8]
// to dword[+0x124] (0x934df4); then edx = i16[+0xda] << 8, eax = i16[+0x14c] << 6 and
// ecx = ((i16[+0x114] << 6) * fraction >> 14) + edx + dword[+0xf4] + eax (flags of that last add).
// ebp is saved and restored.
// FUNCTION: GP4 0x00553c87
void __cdecl FUN_00553c87(gp4::Regs* r) {
    const uint32_t record = r->edi;

    uint32_t reach = static_cast<uint32_t>(GP4_FIELD(uint16_t, record, 0x14a)) << 6;
    if (static_cast<int32_t>(reach) >= GP4_FIELD(int32_t, record, 0x10c)) {
        reach = GP4_FIELD(uint32_t, record, 0x10c);
    }
    int32_t length = GP4_FIELD(int32_t, record, 0x10c);
    if (length == 0) {
        length = 1;
    }
    int32_t remainder;
    const int32_t fraction =
        gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(reach)) << 14, length, &remainder);

    const uint32_t x0 = (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xd6))) << 8) +
                        GP4_FIELD(uint32_t, record, 0xf0);
    const uint32_t dx = GP4_FIELD(uint32_t, record, 0x120) - x0;
    GP4_GLOBAL(uint32_t, 0x00934df0) =
        static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(dx)) * fraction) >> 14) + x0;

    const uint32_t y0 = (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xde))) << 8) +
                        GP4_FIELD(uint32_t, record, 0xf8);
    const uint32_t dy = GP4_FIELD(uint32_t, record, 0x124) - y0;
    GP4_GLOBAL(uint32_t, 0x00934df4) =
        static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(dy)) * fraction) >> 14) + y0;

    const uint32_t height = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x114))) << 6;
    const uint32_t base = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xda))) << 8;
    const uint32_t lift = static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(height)) * fraction) >> 14) +
                          base + GP4_FIELD(uint32_t, record, 0xf4);
    const uint32_t trim = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x14c))) << 6;

    r->eax = trim;
    r->ecx = lift + trim;
    r->edx = base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(lift, trim));
}
GP4_IMPL(0x00553c87, FUN_00553c87, "regs(edi:ptr[0x154]:u8{0,0,0,0,0,0,1,0x40,0x80,0xff}) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
