#include <gp4/gp4.h>
#include <gp4/flags.h>

// The three callees are empty routines (a bare `ret`); one helper per call site keeps the three
// distinct calls.
static __declspec(noinline) void CallFlaggedStub() {
    gp4::cdecl_<0x0051b644, void>();
}
static __declspec(noinline) void CallRowStub() {
    gp4::cdecl_<0x0051b5a4, void>();
}
static __declspec(noinline) void CallLevelStub() {
    gp4::cdecl_<0x0051b5a4, void>();
}

// pushal/popal routine over one 0x20-byte entry of the texture table at 0x6975ec: the entry is
// 0x6975ec plus the offset word 0x9e1b20[word [0x9679a4]], its flag word at +0x14 decides the path
// (0x1000 or 0x40: nothing; 4: the empty callee 0x51b644; otherwise the bytes [0x7c4ad1] /
// [0x7c4ad2] are set to 2 for the missing flag bits 8 / 1 and restored afterwards, and the empty
// callee 0x51b5a4 is called once per row, word [entry+6] times with the dword [entry] advancing by
// 0x100 when bit 0x80 is set, else four times). Every register comes back unchanged; the flags are
// those of the last flag writer on the path taken (the `test` of the deciding bit, the final
// `add eax, 0x100` of the row loop, or the final `cmp ecx, 4`).
// FUNCTION: GP4 0x0051b5a5
void __cdecl FUN_0051b5a5(gp4::Regs* r) {
    const uint32_t slot = GP4_GLOBAL(uint16_t, 0x009679a4);
    const uint8_t* entry = reinterpret_cast<const uint8_t*>(
        static_cast<uintptr_t>(static_cast<uint32_t>(GP4_ARRAY(uint16_t, 0x009e1b20)[slot]) + 0x006975ecu));
    const uint32_t attr = *reinterpret_cast<const uint16_t*>(entry + 0x14);
    uint32_t flags;
    if ((attr & 0x1000u) != 0u) {
        flags = gp4::flags::logic16(attr & 0x1000u);
    } else if ((attr & 4u) != 0u) {
        flags = gp4::flags::logic16(attr & 4u);
        CallFlaggedStub();
    } else if ((attr & 0x40u) != 0u) {
        flags = gp4::flags::logic16(attr & 0x40u);
    } else {
        const uint32_t saved = GP4_GLOBAL(uint32_t, 0x007c4ad1);
        if ((attr & 8u) == 0u) GP4_GLOBAL(uint8_t, 0x007c4ad1) = 2u;
        if ((attr & 1u) == 0u) GP4_GLOBAL(uint8_t, 0x007c4ad2) = 2u;
        if ((attr & 0x80u) != 0u) {
            uint32_t pos = *reinterpret_cast<const uint32_t*>(entry);
            uint32_t count = *reinterpret_cast<const uint16_t*>(entry + 6);
            do {
                CallRowStub();
                flags = gp4::flags::add32(pos, 0x100u);
                pos += 0x100u;
                --count;
            } while (count != 0u);
        } else {
            uint32_t row = 0u;
            do {
                CallLevelStub();
                ++row;
            } while (row != 4u);
            flags = gp4::flags::sub32(row, 4u);
        }
        GP4_GLOBAL(uint32_t, 0x007c4ad1) = saved;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051b5a5, FUN_0051b5a5, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=entry_0051b5a5")
