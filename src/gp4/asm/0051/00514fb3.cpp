#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the four state bytes 0x0088fd94..97 and sets the dword [0x0088fe14] to 2 while the record in
// esi supplies its byte +0x70 (al) to the binary-to-BCD routine 0x0051fcdb (pushal/popal around it).
// The packed BCD value becomes two ASCII digits stored as dwords: [0x008660d4] = the tens digit + 0x30
// (a space 0x20 when the tens are zero) and [0x008660d8] = the units digit + 0x30; [0x008660d0] = 0x6c.
// Afterwards [0x0088fe14] = -1, bit 6 of the record byte +0x88 is cleared and the byte [0x00954169] is
// set to 0xff. Only al of eax is changed (the byte +0x70); the flags are those of the `and byte
// [esi + 0x88], 0xbf`.
// FUNCTION: GP4 0x00514fb3
void __cdecl FUN_00514fb3(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint8_t, 0x0088fd94) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0u;
    GP4_GLOBAL(uint32_t, 0x0088fe14) = 2u;

    const uint32_t eax = (r->eax & 0xffffff00u) | GP4_FIELD(uint8_t, record, 0x70);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = eax;
    gp4::call_regs(0x0051fcdb, c);
    const uint32_t packed = c.eax;

    const uint32_t tens = (packed & 0xffu) >> 4;
    if (tens == 0u) {
        GP4_GLOBAL(uint32_t, 0x008660d4) = 0x20u;
    } else {
        GP4_GLOBAL(uint32_t, 0x008660d4) = (tens + 0x30u) & 0xffu;
    }
    GP4_GLOBAL(uint32_t, 0x008660d8) = ((packed & 0xfu) + 0x30u) & 0xffu;
    GP4_GLOBAL(uint32_t, 0x008660d0) = 0x6cu;

    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0xffffffffu;
    const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x88) & 0xbfu);
    GP4_FIELD(uint8_t, record, 0x88) = cleared;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;

    r->eax = eax;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x00514fb3, FUN_00514fb3, "regs(eax:u32, esi:ptr[0x90]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
