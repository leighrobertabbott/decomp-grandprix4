#include <gp4/gp4.h>
#include <gp4/flags.h>

// Interpolates between two points of a 0x160-byte track record (edi) by a Q14 fraction and returns the
// lateral offsets. The fraction is min(word[+0x144] << 6, dword[+0x100]) << 14 / dword[+0x100] (a zero
// divisor is replaced by 1). The point X = a + ((dword[+0x118] - a) * fraction >> 14) with
// a = (i16[+0xd4] << 8) + dword[+0xf0] goes to 0x934df0, Y likewise from i16[+0xdc], dword[+0xf8] and
// dword[+0x11c] to 0x934df4. Then edx = i16[+0xd8] << 8, eax = i16[+0x146] << 6 and
// ecx = ((i16[+0x108] << 6) * fraction >> 14) + edx + dword[+0xf4] + eax (flags of that last add).
// ebp is saved and restored.
// FUNCTION: GP4 0x00553bd1
void __cdecl FUN_00553bd1(gp4::Regs* r) {
    const uint32_t record = r->edi;

    uint32_t reach = static_cast<uint32_t>(GP4_FIELD(uint16_t, record, 0x144)) << 6;
    if (static_cast<int32_t>(reach) >= GP4_FIELD(int32_t, record, 0x100)) {
        reach = GP4_FIELD(uint32_t, record, 0x100);
    }
    int32_t length = GP4_FIELD(int32_t, record, 0x100);
    if (length == 0) {
        length = 1;
    }
    int32_t remainder;
    const int32_t fraction =
        gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(reach)) << 14, length, &remainder);

    const uint32_t x0 = (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xd4))) << 8) +
                        GP4_FIELD(uint32_t, record, 0xf0);
    const uint32_t dx = GP4_FIELD(uint32_t, record, 0x118) - x0;
    GP4_GLOBAL(uint32_t, 0x00934df0) =
        static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(dx)) * fraction) >> 14) + x0;

    const uint32_t y0 = (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xdc))) << 8) +
                        GP4_FIELD(uint32_t, record, 0xf8);
    const uint32_t dy = GP4_FIELD(uint32_t, record, 0x11c) - y0;
    GP4_GLOBAL(uint32_t, 0x00934df4) =
        static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(dy)) * fraction) >> 14) + y0;

    const uint32_t height = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x108))) << 6;
    const uint32_t base = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xd8))) << 8;
    const uint32_t lift = static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(height)) * fraction) >> 14) +
                          base + GP4_FIELD(uint32_t, record, 0xf4);
    const uint32_t trim = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x146))) << 6;

    r->eax = trim;
    r->ecx = lift + trim;
    r->edx = base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(lift, trim));
}
GP4_IMPL(0x00553bd1, FUN_00553bd1, "regs(edi:ptr[0x14e]:u8{0,0,0,0,0,0,1,0x40,0x80,0xff}) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
