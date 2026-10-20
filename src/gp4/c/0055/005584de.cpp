#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// R3 stream decoder for a track threshold row (active only in mode 1: [0x0086857c] == 1). esi points
// at seven words: the first is the row index (sign extended, stored into the dword table 0x87059c
// as the new row number), the next four go to the 16-byte rows at 0x868598 +0/+4/+8/+0xc and the
// last two to 0x86c59c/0x86c5a0 of the same row, each sign extended. The row counter [0x870598] is
// post-incremented and must be below 0x3ff (else int3). Every register except esi is preserved
// (pushal/popal); esi advances by 0xe in both modes and the flags are those of that final add.
// FUNCTION: GP4 0x005584de
void __cdecl FUN_005584de(gp4::Regs* r) {
    const uint32_t src = r->esi;
    r->esi = src + 0xeu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(src, 0xeu));
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        return;
    }

    const int32_t row = GP4_GLOBAL(int32_t, 0x00870598);
    GP4_GLOBAL(int32_t, 0x00870598) = row + 1;
    if (row >= 0x3ff) {
        __debugbreak();
    }

    const int16_t* words = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(src));
    GP4_ARRAY(int32_t, 0x0087059c)[words[0]] = row;
    const uint32_t off = static_cast<uint32_t>(row) * 0x10u;
    GP4_FIELD(int32_t, off, 0x00868598) = words[1];
    GP4_FIELD(int32_t, off, 0x0086859c) = words[2];
    GP4_FIELD(int32_t, off, 0x008685a0) = words[3];
    GP4_FIELD(int32_t, off, 0x008685a4) = words[4];
    GP4_FIELD(int32_t, off, 0x0086c59c) = words[5];
    GP4_FIELD(int32_t, off, 0x0086c5a0) = words[6];
}
GP4_IMPL(0x005584de, FUN_005584de, "regs(esi:ptr[0x20]:i16[0..200]) -> (esi, cf, pf, af, zf, sf, of); globals=layout=mode_0086857c+cnt_00870598")
