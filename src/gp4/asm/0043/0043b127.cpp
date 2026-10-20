#include <gp4/gp4.h>

static inline int32_t SegI16(uint32_t seg, uint32_t off) {
    return (int32_t)*reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(seg + off));
}

static inline uint16_t SegU16(uint32_t seg, uint32_t off) {
    return *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(seg + off));
}

// Edge point of a track segment: x/z = (word +4/+8 << 6) + (edge word << 3), height = word +6 + edge
// height word; all scaled << 5 (the height via the 16-bit sum kept in cx, whose upper half carries
// over). Returns the new ecx (before the final << 5).
static inline uint32_t EdgePoint(uint32_t seg, uint32_t ecx, uint32_t xOff, uint32_t hOff, uint32_t zOff,
                                 uint32_t* x, uint32_t* h, uint32_t* z) {
    uint32_t c = (ecx & 0xffff0000u) | (uint16_t)(SegU16(seg, 6) + SegU16(seg, hOff));
    c <<= 3;
    *x = (((uint32_t)SegI16(seg, xOff) << 3) + ((uint32_t)SegI16(seg, 4) << 6)) << 5;
    *z = (((uint32_t)SegI16(seg, zOff) << 3) + ((uint32_t)SegI16(seg, 8) << 6)) << 5;
    c <<= 5;
    *h = c;
    return c;
}

// The 0x34-byte entry slot at 0x0063db8c (kept out of line so the fields stay relative to the base).
static __declspec(noinline) uint8_t* EntrySlot(uint32_t index) {
    return GP4_ARRAY(uint8_t, 0x0063db8c) + index * 0x34u;
}

// FUNCTION: GP4 0x0043b127
// pushal/popal routine for the object in esi (x/z at +0/+8, segment byte offset +0x64 into the 0x160-byte
// ring at 0x008943a0). 0x0040460a maps the object into the segment frame; the sign of the mapped x selects
// the left (+0xb8/+0xbc/+0xc0) or right (+0xba/+0xbe/+0xc2) edge. The edge point of the segment goes to
// 0x608290/94/98 and that of the next record (+0x160) to 0x623e30/34/38; 0x00401315 gives the edge
// direction and 0x0043b070 (edx = -1 left, 0 right) tests the object against it. When it reports CF = 1,
// the counter [0x609864] appends a zero dword at 0x609870 + count and slot 0 of 0x63db8c receives the
// first edge point, the direction +- 0x4000 (<< 16) at +0x10, 100000 at +0x28, 0x41 at +0x2c and zeros.
// Flags: those of 0x0043b070 on the early exit, else of "add edi, 0x63db8c" (all clear).
void __cdecl FUN_0043b127(gp4::Regs* r) {
    uint8_t* obj = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.edi = GP4_FIELD(uint32_t, obj, 0x64) + 0x008943a0u;
    GP4_GLOBAL(uint32_t, 0x00623e30) = GP4_FIELD(uint32_t, obj, 0);
    s.eax = GP4_FIELD(uint32_t, obj, 8);
    GP4_GLOBAL(uint32_t, 0x00623e38) = s.eax;
    gp4::call_regs(0x0040460a, s);

    const bool left = GP4_GLOBAL(int32_t, 0x00934df0) < 0;
    const uint32_t xOff = left ? 0xb8u : 0xbau;
    const uint32_t hOff = left ? 0xbcu : 0xbeu;
    const uint32_t zOff = left ? 0xc0u : 0xc2u;

    uint32_t x0, h0, z0;
    s.ecx = EdgePoint(s.edi, s.ecx, xOff, hOff, zOff, &x0, &h0, &z0);
    GP4_GLOBAL(uint32_t, 0x00608290) = x0;
    GP4_GLOBAL(uint32_t, 0x00608294) = h0;
    GP4_GLOBAL(uint32_t, 0x00608298) = z0;

    s.edi += 0x160u;
    uint32_t x1, h1, z1;
    s.ecx = EdgePoint(s.edi, s.ecx, xOff, hOff, zOff, &x1, &h1, &z1);
    GP4_GLOBAL(uint32_t, 0x00623e30) = x1;
    GP4_GLOBAL(uint32_t, 0x00623e34) = h1;
    GP4_GLOBAL(uint32_t, 0x00623e38) = z1;

    s.eax = GP4_GLOBAL(uint32_t, 0x00623e30) - GP4_GLOBAL(uint32_t, 0x00608290);
    s.edx = GP4_GLOBAL(uint32_t, 0x00623e38) - GP4_GLOBAL(uint32_t, 0x00608298);
    gp4::call_regs(0x00401315, s);
    s.edx = left ? 0xffffffffu : 0u;
    gp4::call_regs(0x0043b070, s);
    if ((s.eflags & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
        return;
    }

    const uint32_t heading = (left ? s.eax + 0x4000u : s.eax - 0x4000u) << 16;

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00609864);
    *reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x00609870) + count) = 0u;
    GP4_GLOBAL(uint32_t, 0x00609864) = count + 1u;

    uint8_t* slot = EntrySlot(0u);
    GP4_FIELD(uint32_t, slot, 0x10) = heading;
    GP4_FIELD(uint32_t, slot, 0x0c) = 0u;
    GP4_FIELD(uint32_t, slot, 0x14) = 0u;
    GP4_FIELD(uint32_t, slot, 0x00) = GP4_GLOBAL(uint32_t, 0x00608290);
    GP4_FIELD(uint32_t, slot, 0x04) = GP4_GLOBAL(uint32_t, 0x00608294);
    GP4_FIELD(uint32_t, slot, 0x08) = GP4_GLOBAL(uint32_t, 0x00608298);
    GP4_FIELD(uint32_t, slot, 0x18) = 0u;
    GP4_FIELD(uint32_t, slot, 0x1c) = 0u;
    GP4_FIELD(uint32_t, slot, 0x20) = 0u;
    GP4_FIELD(uint32_t, slot, 0x24) = 0u;
    GP4_FIELD(uint32_t, slot, 0x28) = 100000u;
    GP4_FIELD(uint32_t, slot, 0x2c) = 0x41u;
    GP4_FIELD(uint32_t, slot, 0x30) = 0u;

    r->eflags &= ~0x8d5u;
}
GP4_IMPL(0x0043b127, FUN_0043b127, "regs(ecx:u32, esi:ptr[0x70]:layout=obj_00424e1b+obj_0043b127) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00405a12+seg_00424e1b+state_004250b9+seg_0043b127+rate_00603854")
