#include <gp4/gp4.h>
#include <gp4/flags.h>

// Word of the table at 0x88f572 at the signed byte offset `offset`.
static __declspec(noinline) uint16_t tableWord(int32_t offset) {
    return *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(0x0088f572 + offset));
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

// FUNCTION: GP4 0x0055ad9b
// cx = count (decremented first), dx = base index added to a random draw; edi points to the
// word that is the nibble number for 0x0055adfe. While cx >= 1 a random value is drawn
// (0x004015dc) and masked with 3 (cx < 4) or 7 until it is <= cx. The word table at 0x88f572 is
// then read at 2 * (draw + dx) (signed 16-bit byte offset) and its low nibble is stored by
// 0x0055adfe into nibble [edi]; when that nibble did not change (ZF) and cx >= 1 the draw is
// repeated. Flags: those of 0x0055adfe when it changed the nibble, else of `cmp cx, 1`.
void __cdecl FUN_0055ad9b(gp4::Regs* r) {
    gp4::Regs s = *r;
    const uint16_t* wordPtr = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(s.edi));

    *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(0x00874b4c)) = static_cast<uint16_t>(s.edx);
    setLow16(s.ecx, s.ecx - 1u);
    setLow16(s.eax, s.ecx);

    uint32_t flags = gp4::flags::sub16(s.ecx, 1u);
    bool skipDraw = static_cast<int16_t>(s.ecx) < 1;
    for (;;) {
        if (!skipDraw) {
            const bool small = static_cast<int16_t>(s.ecx) < 4;
            do {
                gp4::call_regs(0x004015dc, s);
                setLow16(s.eax, s.eax & (small ? 3u : 7u));
            } while (static_cast<int16_t>(s.eax) > static_cast<int16_t>(s.ecx));
        }
        skipDraw = false;

        const uint32_t sum = (s.eax + GP4_GLOBAL(uint16_t, 0x00874b4c)) & 0xffffu;
        const uint32_t doubled = (sum << 1) & 0xffffu;
        const int32_t offset = static_cast<int16_t>(doubled);
        s.eax = static_cast<uint32_t>(offset);
        setLow16(s.eax, tableWord(offset));
        setLow16(s.edx, *wordPtr);
        gp4::call_regs(0x0055adfe, s);
        flags = s.eflags & gp4::flags::ARITH;
        if ((flags & gp4::flags::ZF) == 0u) {
            break;
        }
        flags = gp4::flags::sub16(s.ecx, 1u);
        if (static_cast<int16_t>(s.ecx) < 1) {
            break;
        }
    }

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055ad9b, FUN_0055ad9b, "regs(eax:u32, ecx:u32[0..10], edx:u32[0..0x18], edi:ptr[0x10]:u16[0..0x3f]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=seed_0062d1b4+nib_00870618")
