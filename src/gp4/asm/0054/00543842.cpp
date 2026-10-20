#include <gp4/gp4.h>
#include <gp4/flags.h>

// Pointer stored at slot `slot` of the pointer array at +0xc of a block.
static __declspec(noinline) uint32_t SlotPointer(uint32_t block, uint32_t slot) {
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(block + slot * 4u + 0xcu));
}

// FUNCTION: GP4 0x00543842
// Only acts when edx == 2. esi points at an object whose dword at +0xe is a block; the block's
// dword +4 (a count) must be non-zero and the block's pointer array at +0xc is indexed by
// [0xabc088]. If the selected record does not yet have bit 7 of its byte +9 set, 0x5438a0 is
// called (it clears that bit in the forty 0x20-byte records at 0x830bb4) and the bit is then set
// on the selected record. eax, ebx and edi keep the values the original leaves; the flags are
// those of the last compare/test/or.
void __cdecl FUN_00543842(gp4::Regs* r) {
    const uint32_t kind = r->edx;
    uint32_t flags = gp4::flags::sub32(kind, 2u);
    if (kind == 2u) {
        const uint32_t block = GP4_FIELD(uint32_t, r->esi, 0xe);
        const uint32_t slot = GP4_GLOBAL(uint32_t, 0x00abc088);
        const uint32_t count = GP4_FIELD(uint32_t, block, 4);
        r->edi = block;
        r->eax = slot;
        r->ebx = count;
        flags = gp4::flags::sub32(count, 0u);
        if (count != 0u) {
            const uint32_t record = SlotPointer(block, slot);
            r->eax = record;
            const uint8_t before = GP4_FIELD(uint8_t, record, 9);
            flags = gp4::flags::logic8(before & 0x80u);
            if ((before & 0x80u) == 0u) {
                gp4::Regs c = *r;
                gp4::call_regs(0x005438a0, c);
                r->eax = c.eax;
                r->ebx = c.ebx;
                r->edi = c.edi;
                const uint8_t after = static_cast<uint8_t>(GP4_FIELD(uint8_t, c.eax, 9) | 0x80u);
                GP4_FIELD(uint8_t, c.eax, 9) = after;
                flags = gp4::flags::logic8(after);
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00543842, FUN_00543842, "regs(eax:u32, ebx:u32, edi:u32, edx:u32{=2,2,2,0,1,3}, esi:ptr[0x20]:layout=obj_00543842) -> (eax, ebx, edi, cf, pf, af, zf, sf, of); globals=layout=sel_00543842+recs_005438a0")
