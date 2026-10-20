#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: looks up the car record selected by the index in [0xa69878] (clamped to
// 0..21 by 0x0040838c) and publishes its driver (byte +0x7c & 0x3f, minus one, at least 0) and team
// (byte +0x73 minus one, at least 0) data: the driver index back into [0xa69878], the driver name
// slot (0x18 bytes each from 0x7c2b3c) into [0xa6987c], the per-driver byte at 0x7c2a5e into
// [0xa69884], the team name slot (0xd bytes each from 0x7c2d4c) into [0xa69880] and a flag set when
// the team's word (every 4 bytes from 0x7c2acc) is 0x335 into [0xa69888]. Every register is
// restored; the flags are those of the last compare of the team word (against 0x335, or against
// 0x334 when it differs).
// FUNCTION: GP4 0x00516142
void __cdecl FUN_00516142(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x0040838c, c);
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.eax));

    int32_t driver = static_cast<int32_t>(car[0x7c] & 0x3fu) - 1;
    if (driver < 0) driver = 0;
    GP4_GLOBAL(int32_t, 0x00a69878) = driver;
    GP4_GLOBAL(uint32_t, 0x00a6987c) =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c2b3c))) +
        static_cast<uint32_t>(driver) * 0x18u;
    GP4_GLOBAL(uint32_t, 0x00a69884) = GP4_ARRAY(uint8_t, 0x007c2a5e)[driver];

    const int32_t teamRaw = static_cast<int32_t>(car[0x73]) - 1;
    int32_t team = teamRaw;
    if (team < 0) team = 0;
    GP4_GLOBAL(uint32_t, 0x00a69880) =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c2d4c))) +
        static_cast<uint32_t>(team) * 0xdu;

    const uint32_t word =
        *reinterpret_cast<const uint16_t*>(GP4_ARRAY(uint8_t, 0x007c2acc) + teamRaw * 4);
    uint32_t flags;
    if (word == 0x335u) {
        GP4_GLOBAL(uint32_t, 0x00a69888) = 1u;
        flags = gp4::flags::sub32(word, 0x335u);
    } else {
        GP4_GLOBAL(uint32_t, 0x00a69888) = 0u;
        flags = gp4::flags::sub32(word, 0x334u);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516142, FUN_00516142, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+carno_0066ff6c+idx_005162fd")
