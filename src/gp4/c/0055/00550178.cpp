#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00550178
// Initialises [0x934eac] consecutive 0x34-byte records from 0x93523c. Each record is filled by
// 0x550798, gets bit 0 set in its first dword, and its +8 dword is advanced by
// (random & 0xf) * (the dword at +0x14), the random value coming from 0x54ff84.
// Exit state: esi = end of the records, ecx = 0, ebx = the last record's +0x14, edx:eax = the last
// signed product; the flags are those of the final `dec ecx` (CF kept from `add esi, 0x34`).
// Afterwards the byte 0x6f3b7b is cleared.
void __cdecl FUN_00550178(gp4::Regs* r) {
    uint32_t record = 0x0093523cu;
    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x00934eac);
    uint32_t low = r->eax;
    uint32_t high = r->edx;
    uint32_t factor = r->ebx;
    uint32_t flags;

    do {
        gp4::Regs c = {};
        c.esi = record;
        gp4::call_regs(0x00550798, c);

        GP4_FIELD(uint32_t, record, 0) |= 1u;

        c = {};
        gp4::call_regs(0x0054ff84, c);

        const uint32_t random = c.eax & 0xfu;
        factor = GP4_FIELD(uint32_t, record, 0x14);
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(random)) * static_cast<int32_t>(factor);
        low = static_cast<uint32_t>(product);
        high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        GP4_FIELD(uint32_t, record, 8) += low;

        flags = gp4::flags::add32(record, 0x34u);
        record += 0x34u;
        --remaining;
        flags = gp4::flags::dec32(remaining + 1u, flags);
    } while (remaining != 0u);

    GP4_GLOBAL(uint8_t, 0x006f3b7b) = 0u;

    r->eax = low;
    r->ebx = factor;
    r->ecx = 0u;
    r->edx = high;
    r->esi = record;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00550178, FUN_00550178, "regs() -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+box_00550798+cnt_00550178")
