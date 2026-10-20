#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream decoder for a signed word pair limit (mode byte 0x0086857c == 1, else only esi += 2).
// Reads one word from esi (esi += 2). For kind cl != 0x15 the word is the new lower limit
// [0x8705fa] and is clamped to the upper limit [0x8705fc] (signed); for cl == 0x15 it is the new
// upper limit [0x8705fc], clamped to [0x8705fa]. The clamped value is stored to the four words
// 0x88f36a / 0x88f372 / 0x88f396 / 0x88f39e and left in ax (upper half of eax unchanged). Flags:
// `add esi, 2` for any other mode, else the signed `cmp ax, word [limit]`.
// FUNCTION: GP4 0x0055721e
void __cdecl FUN_0055721e(gp4::Regs* r) {
    const uint32_t source = r->esi;
    r->esi = source + 2u;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 2u));
        return;
    }

    uint16_t value = GP4_FIELD(uint16_t, source, 0);
    uint32_t arith;
    if ((r->ecx & 0xffu) != 0x15u) {
        GP4_GLOBAL(uint16_t, 0x008705fa) = value;
        const uint16_t limit = GP4_GLOBAL(uint16_t, 0x008705fc);
        arith = gp4::flags::sub16(value, limit);
        if (static_cast<int16_t>(value) > static_cast<int16_t>(limit)) {
            value = GP4_GLOBAL(uint16_t, 0x008705fc);
        }
        GP4_GLOBAL(uint16_t, 0x0088f36a) = value;
        GP4_GLOBAL(uint16_t, 0x0088f372) = value;
        GP4_GLOBAL(uint16_t, 0x0088f396) = value;
        GP4_GLOBAL(uint16_t, 0x0088f39e) = value;
    } else {
        GP4_GLOBAL(uint16_t, 0x008705fc) = value;
        const uint16_t limit = GP4_GLOBAL(uint16_t, 0x008705fa);
        arith = gp4::flags::sub16(value, limit);
        if (static_cast<int16_t>(value) > static_cast<int16_t>(limit)) {
            value = GP4_GLOBAL(uint16_t, 0x008705fa);
        }
        GP4_GLOBAL(uint16_t, 0x0088f396) = value;
        GP4_GLOBAL(uint16_t, 0x0088f39e) = value;
        GP4_GLOBAL(uint16_t, 0x0088f36a) = value;
        GP4_GLOBAL(uint16_t, 0x0088f372) = value;
    }
    r->eax = (r->eax & 0xffff0000u) | value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055721e, FUN_0055721e, "regs(eax:u32, ecx:u32{0x15,0x15,0x00,0x57,0xffffff15}, esi:ptr[8]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
