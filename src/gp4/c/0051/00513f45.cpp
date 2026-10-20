#include <gp4/gp4.h>
#include <gp4/flags.h>

// Raises the event number just stored in [0x88ff30] through the dispatcher 0x514152 with the
// current registers and flags; its answer is the flags it leaves.
static inline void RaiseEvent(const gp4::Regs* r, uint32_t& flags) {
    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(r->eflags, flags);
    gp4::call_regs(0x00514152, c);
    flags = c.eflags;
}

// Camera/replay event sequence for the record in esi. When the byte flag [0x7c2ea9] has its sign
// bit set it raises events 2 and 3. Otherwise it raises event 3 and, unless the dword +0x38 of the
// record is the 0x10000000 "none" marker, publishes it in [0x893cec], stores +0x28 - +0x38 in
// [0x866078] (also left in eax) and raises events 0x12 and 0x13. [0x893cec] is cleared on exit; the
// flags are those of the last compare/subtract or dispatcher result.
// FUNCTION: GP4 0x00513f45
void __cdecl FUN_00513f45(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = gp4::flags::logic8(mode);
    if ((mode & 0x80u) != 0u) {
        GP4_GLOBAL(uint32_t, 0x0088ff30) = 2u;
        RaiseEvent(r, flags);
        GP4_GLOBAL(uint32_t, 0x0088ff30) = 3u;
        RaiseEvent(r, flags);
    } else {
        GP4_GLOBAL(uint32_t, 0x0088ff30) = 3u;
        RaiseEvent(r, flags);
        const uint32_t target = GP4_FIELD(uint32_t, record, 0x38);
        flags = gp4::flags::sub32(target, 0x10000000u);
        if (target != 0x10000000u) {
            GP4_GLOBAL(uint32_t, 0x00893cec) = target;
            const uint32_t source = GP4_FIELD(uint32_t, record, 0x28);
            const uint32_t diff = source - target;
            flags = gp4::flags::sub32(source, target);
            r->eax = diff;
            GP4_GLOBAL(uint32_t, 0x00866078) = diff;
            GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x12u;
            RaiseEvent(r, flags);
            GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x13u;
            RaiseEvent(r, flags);
        }
    }
    GP4_GLOBAL(uint32_t, 0x00893cec) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00513f45, FUN_00513f45, "regs(eax:u32, ebx:u32, ecx:u32, edi:u32, esi:ptr[0x40]:u32{=0x10000000,1,2,3,5,7,0x1000,0xfffff000}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=player_0040f8d9+disp_005141b8+events_00514152+times_00513f45")
