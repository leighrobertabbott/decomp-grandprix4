#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00551155
// Loads the per-segment values of the 0x160-byte record at edi into the globals of the drawing
// code: clears byte 0x871c79, stores the byte +0x2b in 0x871c90, the word +0 in 0x875e7c, the
// signed word +4 << 6 in 0x875e60/0x875e6c, the signed word +8 << 6 in 0x875e68/0x875e74 and the
// signed word +6 << 3 in 0x875e64/0x875e70. esi leaves holding the byte +0x2b, eax the last value
// (word +6 << 3); the flags are those of that final `shl eax, 3`.
void __cdecl FUN_00551155(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(uint8_t, 0x00871c79) = 0u;
    const uint32_t flagByte = record[0x2b];
    GP4_GLOBAL(uint32_t, 0x00871c90) = flagByte;
    GP4_GLOBAL(uint16_t, 0x00875e7c) = *reinterpret_cast<const uint16_t*>(record);

    const int32_t x = *reinterpret_cast<const int16_t*>(record + 4);
    const uint32_t scaledX = static_cast<uint32_t>(x) << 6;
    GP4_GLOBAL(uint32_t, 0x00875e60) = scaledX;
    GP4_GLOBAL(uint32_t, 0x00875e6c) = scaledX;

    const int32_t y = *reinterpret_cast<const int16_t*>(record + 8);
    const uint32_t scaledY = static_cast<uint32_t>(y) << 6;
    GP4_GLOBAL(uint32_t, 0x00875e68) = scaledY;
    GP4_GLOBAL(uint32_t, 0x00875e74) = scaledY;

    const uint32_t z = static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(record + 6)));
    const uint32_t scaledZ = z << 3;
    GP4_GLOBAL(uint32_t, 0x00875e64) = scaledZ;
    GP4_GLOBAL(uint32_t, 0x00875e70) = scaledZ;

    // shl eax, 3: CF = last bit shifted out; OF = bit31((z << 2) ^ result) (pinned oracle model);
    // AF = 0.
    const uint32_t shiftFlags = ((z >> 29) & 1u)
        | gp4::flags::szp(scaledZ, 32)
        | ((((z << 2) ^ scaledZ) & 0x80000000u) != 0u ? static_cast<uint32_t>(gp4::flags::OF) : 0u);

    r->eax = scaledZ;
    r->esi = flagByte;
    r->eflags = gp4::flags::merge(r->eflags, shiftFlags);
}
GP4_IMPL(0x00551155, FUN_00551155, "regs(edi:ptr[0x40]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
