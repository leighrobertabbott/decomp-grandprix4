#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the event number in [0x88ff30] and raises it through the dispatcher 0x514152 with the
// current registers; the dispatcher keeps every register and leaves its flags in r.
static inline void Raise(gp4::Regs* r, uint32_t event) {
    GP4_GLOBAL(uint32_t, 0x0088ff30) = event;
    gp4::call_regs(0x00514152, *r);
}

// Event sequence for the record in esi (the caller 0x513ce2 runs it for the player's car): raises
// events 0, 1, 4, 5, 6, 7, 8, 0x1b, 9, 10, (0xd unless +0x82 bit 3), 0xf, 0x10, 0x15, 0x16, 0x13,
// 0x19 and 0x1a, resetting the globals 0x88fda6 (byte), 0x893e0c, 0x893e08, 0x88fdac, 0x88fdb4,
// 0x88fdb8 and 0x893e04 to their "none" values in between. The flags are the dispatcher's last.
// FUNCTION: GP4 0x00513de3
void __cdecl FUN_00513de3(gp4::Regs* r) {
    const uint32_t record = r->esi;
    GP4_GLOBAL(uint8_t, 0x0088fda6) = 0x7fu;
    Raise(r, 0u);
    Raise(r, 1u);
    Raise(r, 4u);
    GP4_GLOBAL(uint32_t, 0x00893e0c) = 0xffffffffu;
    Raise(r, 5u);
    GP4_GLOBAL(uint32_t, 0x00893e08) = 0xffffffffu;
    Raise(r, 6u);
    GP4_GLOBAL(uint8_t, 0x0088fda6) = 0x80u;
    GP4_GLOBAL(uint32_t, 0x0088fdac) = 0x80000000u;
    Raise(r, 7u);
    GP4_GLOBAL(uint32_t, 0x0088fdb4) = 0x80000000u;
    Raise(r, 8u);
    GP4_GLOBAL(uint32_t, 0x0088fdb8) = 0x80000000u;
    Raise(r, 0x1bu);
    Raise(r, 9u);
    Raise(r, 10u);
    if ((GP4_FIELD(uint8_t, record, 0x82) & 8u) == 0u) {
        Raise(r, 0xdu);
    }
    Raise(r, 0xfu);
    Raise(r, 0x10u);
    Raise(r, 0x15u);
    Raise(r, 0x16u);
    Raise(r, 0x13u);
    Raise(r, 0x19u);
    GP4_GLOBAL(uint32_t, 0x00893e04) = 0xffffffffu;
    Raise(r, 0x1au);
}
GP4_IMPL(0x00513de3, FUN_00513de3, "regs(eax:u32, ebx:u32, ecx:u32, edi:u32, esi:ptr[0x90]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=player_0040f8d9+disp_005141b8+events_00514152+times_00513f45")
