#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-entry refresh (esi = the item record, edi = the readout buffer): after the empty hook 0x00515b63
// (eax = [0x00891bd0 + 4 * 0x1f] is loaded first), computes the remaining budget
// max(0, [0x007c2e98] - 0x0066ec4a[k]) with k = min(((byte +0x7c) & 0x3f) - 1, 0x15) (signed) into
// [0x00866088], increments the byte [0x0088fd94], prints the value through 0x00510e71 (two digits at
// edi), then clears bit 7 of byte +0x8b and sets 0x00954169 to 0xff. Registers come from the printing
// routine, the flags are those of the final `and byte [esi+0x8b], 0x7f`.
// FUNCTION: GP4 0x00515aea
void __cdecl FUN_00515aea(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.eax = GP4_ARRAY(uint32_t, 0x00891bd0)[0x1f];
    gp4::call_regs(0x00515b63, c);

    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi));
    int32_t row = static_cast<int32_t>(record[0x7c] & 0x3fu) - 1;
    if (row > 0x15) {
        row = 0x15;
    }
    const uint32_t cost = GP4_ARRAY(uint8_t, 0x0066ec4a)[row];
    int32_t budget = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x007c2e98) - cost);
    if (budget < 0) {
        budget = 0;
    }
    c.eax = static_cast<uint32_t>(budget);
    GP4_GLOBAL(uint32_t, 0x00866088) = c.eax;
    GP4_GLOBAL(uint8_t, 0x0088fd94) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0088fd94) + 1u);
    gp4::call_regs(0x00510e71, c);

    uint8_t* state = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi)) + 0x8b;
    const uint32_t cleared = *state & 0x7fu;
    *state = static_cast<uint8_t>(cleared);
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(cleared));
    *r = c;
}
GP4_IMPL(0x00515aea, FUN_00515aea, "regs(esi:ptr[0x94]:bytes, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+hudpos_00514def+ctabs_005153db")
