#include <gp4/gp4.h>

// pushal ... popal: every register comes back, only the flags of the closing "cmp edi, [0x00866ef8]"
// (equal: ZF and PF set) remain. The pit cursor, pit-entry and pit-exit records are copied to
// 0x00604424 / 0x00604428 / 0x0060442c, 0x0040d8a5 runs on them, and then each pit segment record
// from [0x00891500] up to [0x00866ef8] (stride 0x160) fills its 8-byte row at
// 0x00604435 + 8 * word[rec+0x7a]: word +4 = the track position (word[rec+0x1a] & 0x7ff) mapped by
// 0x00405985, minus the lap length [0x00868558] when it is not below it (signed), word +6 = cx << 8.
// FUNCTION: GP4 0x0040d9e6
void __cdecl FUN_0040d9e6(gp4::Regs* r) {
    gp4::Regs s = *r;
    GP4_GLOBAL(uint32_t, 0x00604424) = GP4_GLOBAL(uint32_t, 0x00891500);
    s.eax = GP4_GLOBAL(uint32_t, 0x00866f00);
    GP4_GLOBAL(uint32_t, 0x00604428) = s.eax;
    s.eax = GP4_GLOBAL(uint32_t, 0x00866f04);
    GP4_GLOBAL(uint32_t, 0x0060442c) = s.eax;
    gp4::call_regs(0x0040d8a5, s);

    s.edi = GP4_GLOBAL(uint32_t, 0x00891500);
    do {
        uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.edi));
        const uint32_t row = static_cast<uint32_t>(GP4_FIELD(uint16_t, segment, 0x7a)) * 8u + 0x00604435u;
        s.eax = (s.eax & 0xffff0000u) | (GP4_FIELD(uint16_t, segment, 0x1a) & 0x7ffu);
        s.ecx = row & 0xffff0000u;
        gp4::call_regs(0x00405985, s);

        uint16_t position = static_cast<uint16_t>(s.eax);
        const uint16_t shifted = static_cast<uint16_t>(static_cast<uint16_t>(s.ecx) << 8);
        s.eax = (s.eax & 0xffff0000u) | shifted;
        const int16_t length = GP4_GLOBAL(int16_t, 0x00868558);
        if (static_cast<int16_t>(position) >= length) {
            position = static_cast<uint16_t>(position - static_cast<uint16_t>(length));
        }
        s.edx = (s.edx & 0xffff0000u) | position;
        s.ecx = row;
        uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(row));
        GP4_FIELD(uint16_t, entry, 4) = position;
        GP4_FIELD(uint16_t, entry, 6) = shifted;
        s.edi += 0x160u;
    } while (s.edi != GP4_GLOBAL(uint32_t, 0x00866ef8));
    // cmp edi, [0x00866ef8] with equal operands
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040d9e6, FUN_0040d9e6, "regs(edx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_0040d8a5+pit_0040d9e6")
