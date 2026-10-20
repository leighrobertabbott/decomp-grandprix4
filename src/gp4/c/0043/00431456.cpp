#include <gp4/gp4.h>

// FUNCTION: GP4 0x00431456
// Walks the 0xffff-terminated list of record offsets at 0x0066fe3c and, for each
// record of the 0x41c-stride array at 0x0066fef0, clears flag bit 0 at +0x203 and
// sets it again when the record is idle (word +0xea == 0, byte +0xfe bit 6 clear),
// its block (+0x1c) has bit 0x20 at +0x51, and the signed word difference
// (+0x90 - +0x94, negated unless block bit 8 is set) reaches the word at 0x00603ba8.
void __cdecl FUN_00431456(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t esi = r->esi;
    uint32_t edi = r->edi;
    uint32_t ebx = 0x0066fe3a;
    uint8_t* const records = GP4_ARRAY(uint8_t, 0x0066fef0);

    for (;;) {
        ebx += 2;
        const uint16_t entry = *reinterpret_cast<const uint16_t*>(ebx);
        eax = (eax & 0xffff0000u) | entry;
        if (entry == 0xffffu) {
            break;
        }
        esi = reinterpret_cast<uint32_t>(records) + entry;
        uint8_t* const rec = reinterpret_cast<uint8_t*>(esi);

        GP4_FIELD(uint8_t, rec, 0x203) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x203) & 0xfeu);
        if (GP4_FIELD(uint16_t, rec, 0xea) != 0) {
            continue;
        }
        if ((GP4_FIELD(uint8_t, rec, 0xfe) & 0x40u) != 0) {
            continue;
        }
        edi = GP4_FIELD(uint32_t, rec, 0x1c);
        const uint8_t blockFlags = *reinterpret_cast<const uint8_t*>(edi + 0x51);
        if ((blockFlags & 0x20u) == 0) {
            continue;
        }
        uint16_t diff = static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x90) - GP4_FIELD(uint16_t, rec, 0x94));
        eax = (eax & 0xffff0000u) | diff;
        if ((blockFlags & 0x08u) == 0) {
            diff = static_cast<uint16_t>(0u - diff);
            eax = (eax & 0xffff0000u) | diff;
        }
        if (static_cast<int16_t>(diff) < GP4_GLOBAL(int16_t, 0x00603ba8)) {
            continue;
        }
        GP4_FIELD(uint8_t, rec, 0x203) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x203) | 1u);
    }

    r->eax = eax;
    r->ebx = ebx;
    r->esi = esi;
    r->edi = edi;
    // flags of the final `cmp ax, -1` with ax == 0xffff: ZF and PF set, the rest clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00431456, FUN_00431456, "regs(eax:u32, esi:u32, edi:u32) -> (eax, ebx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_00431456")
