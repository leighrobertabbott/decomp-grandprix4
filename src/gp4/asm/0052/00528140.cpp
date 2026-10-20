#include <gp4/gp4.h>
#include <gp4/flags.h>

// Registers the frame object in eax (edi) in the 8-entry slot table at 0x0081cd91 (0x1c bytes each; dword +0 is
// the state key, +4 the owner object). Takes the first free slot (0x0052846f; nothing happens when the table is
// full). Every slot with key 1 owned by an object whose first dword equals the object's own is cleared. Then a
// slot with key 3 (cleared; the new slot becomes key 2, link +0x14 = 0) or else a slot with key 2 (new slot
// key 1, link +0x14 = that slot and its +0x10 set to 1) or neither (new slot key 2, no link) is looked up
// through 0x0052848b. The slot receives the owner (+4), the object's dwords +0xc, +8 and +4 (as +0xc, +0x10,
// +8) and bit 0 of +0x18. An object without bit 7 in its flag dword +0x14 gets its frame bounding box computed
// (0x00528379) and the bit set; with bit 6 the frame file is closed (0x00528450). The registers and flags are
// whatever the last callee or flag writer leaves.
// FUNCTION: GP4 0x00528140
void __cdecl FUN_00528140(gp4::Regs* r) {
    const uint32_t object = r->eax;
    gp4::Regs c = *r;
    c.edi = object;
    gp4::call_regs(0x0052846f, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax == 0u) {
        *r = c;
        return;
    }
    const uint32_t slot = c.eax;
    c.esi = slot;

    for (;;) {
        c.eax = GP4_FIELD(uint32_t, object, 0);
        c.ebx = 1u;
        gp4::call_regs(0x0052848b, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
        if (c.eax == 0u) {
            break;
        }
        GP4_FIELD(uint32_t, c.eax, 0) = 0u;
    }

    c.eax = GP4_FIELD(uint32_t, object, 0);
    c.ebx = 3u;
    gp4::call_regs(0x0052848b, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax != 0u) {
        GP4_FIELD(uint32_t, c.eax, 0) = 0u;
        GP4_FIELD(uint32_t, slot, 0) = 2u;
        GP4_FIELD(uint32_t, slot, 0x14) = 0u;
    } else {
        c.eax = GP4_FIELD(uint32_t, object, 0);
        c.ebx = 2u;
        gp4::call_regs(0x0052848b, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
        if (c.eax == 0u) {
            GP4_FIELD(uint32_t, slot, 0) = 2u;
            GP4_FIELD(uint32_t, slot, 0x14) = 0u;
        } else {
            GP4_FIELD(uint32_t, slot, 0) = 1u;
            GP4_FIELD(uint32_t, slot, 0x14) = c.eax;
            GP4_FIELD(uint32_t, c.eax, 0x10) = 1u;
        }
    }

    GP4_FIELD(uint32_t, slot, 4) = object;
    c.eax = GP4_FIELD(uint32_t, object, 0xc);
    GP4_FIELD(uint32_t, slot, 0xc) = c.eax;
    c.eax = GP4_FIELD(uint32_t, object, 8);
    GP4_FIELD(uint32_t, slot, 0x10) = c.eax;
    c.eax = GP4_FIELD(uint32_t, object, 4);
    GP4_FIELD(uint32_t, slot, 8) = c.eax;
    const uint32_t marked = GP4_FIELD(uint32_t, slot, 0x18) | 1u;
    GP4_FIELD(uint32_t, slot, 0x18) = marked;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(marked));

    const uint32_t flagWord = GP4_FIELD(uint32_t, object, 0x14);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(flagWord & 0x80u));
    if ((flagWord & 0x80u) == 0u) {
        gp4::call_regs(0x00528379, c);
        const uint32_t withBit = GP4_FIELD(uint32_t, object, 0x14) | 0x80u;
        GP4_FIELD(uint32_t, object, 0x14) = withBit;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(withBit));
    }
    const uint32_t closeBit = GP4_FIELD(uint32_t, object, 0x14) & 0x40u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(closeBit));
    if (closeBit != 0u) {
        gp4::call_regs(0x00528450, c);
    }
    *r = c;
}
GP4_IMPL(0x00528140, FUN_00528140, "regs(eax:ptr[0x38]:layout=obj_00528379+key_0052848b) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_0052848b+frames_008174f8")
