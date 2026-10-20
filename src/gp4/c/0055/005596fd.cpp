#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written routine (no stack arguments; clobbers eax/ebx/ecx/edx/esi, keeps edi).
// 1. Moves the 50 words at 0x00921ffc..0x00922060 to 0x008705e8..0x0087064c and clears the source.
// 2. Copies dwords +0xf0/+0xf4/+0xf8 of the record [0x00866f00] to 0x00870640/44/48 (low 11 bits
//    of the first and third cleared when byte 0x00852c38 is zero).
// 3. Calls 0x00559663 (dl = 2, cx = width or 0x280, edi = that record): it leaves dx:ax and cx:bx,
//    two Q14 cos-table products of the record's angle word with a distance it stores in 0x00934df0.
//    dx:ax << 8 is added to 0x00870640 and cx:bx << 8 subtracted from 0x00870648.
// 4. Same call on the record [0x00866f04] (dl = 8, words +0x60/+0x62 first copied to
//    0x00870608/0x00870604): dx:ax goes to 0x00874b24, -(cx:bx) to 0x00874b28.
// 5. Resets the words 0x00870602..0x00870616 and clears bits 4/5 of byte +0x48 of the second record.
static __declspec(noinline) uint32_t move_source(uint32_t src_end, uint32_t dst_end, uint32_t dst) {
    return src_end - dst_end + dst;
}

// FUNCTION: GP4 0x005596fd
void __cdecl FUN_005596fd(gp4::Regs* r) {
    uint16_t* src = reinterpret_cast<uint16_t*>(move_source(0x00922060u, 0x0087064cu, 0x008705e8u));
    uint16_t* dst = GP4_ARRAY(uint16_t, 0x008705e8);
    do {
        *dst = *src;
        *src = 0;
        ++src;
        ++dst;
    } while (dst != GP4_ARRAY(uint16_t, 0x0087064c));

    uint8_t* const first = GP4_GLOBAL(uint8_t*, 0x00866f00);
    GP4_GLOBAL(uint32_t, 0x00870640) = GP4_FIELD(uint32_t, first, 0xf0);
    GP4_GLOBAL(uint32_t, 0x00870644) = GP4_FIELD(uint32_t, first, 0xf4);
    const uint32_t f8 = GP4_FIELD(uint32_t, first, 0xf8);
    GP4_GLOBAL(uint32_t, 0x00870648) = f8;
    if (GP4_GLOBAL(uint8_t, 0x00852c38) == 0) {
        GP4_GLOBAL(uint32_t, 0x00870640) &= 0xfffff800u;
        GP4_GLOBAL(uint32_t, 0x00870648) &= 0xfffff800u;
    }

    gp4::Regs c = {};
    c.eax = f8;
    c.ebx = reinterpret_cast<uint32_t>(src);
    c.ecx = GP4_GLOBAL(uint8_t, 0x00852c38) != 0 ? GP4_GLOBAL(uint32_t, 0x00852c44) : 0x280u;
    c.edx = (r->edx & 0xffffff00u) | 2u;
    c.esi = r->esi;
    c.edi = reinterpret_cast<uint32_t>(first);
    c.ebp = r->ebp;
    c.eflags = r->eflags;
    gp4::call_regs(0x00559663, c);

    GP4_GLOBAL(uint16_t, 0x00874b66) = GP4_GLOBAL(uint16_t, 0x00934df0);
    const uint32_t ax_part = ((c.edx << 16) | (c.eax & 0xffffu)) << 8;
    GP4_GLOBAL(uint32_t, 0x00870640) += ax_part;
    const uint32_t cx_high = c.ecx << 16;
    const uint32_t bx_part = ((c.ebx & 0xffffu) | cx_high) << 8;
    GP4_GLOBAL(uint32_t, 0x00870648) -= bx_part;

    uint8_t* const second = GP4_GLOBAL(uint8_t*, 0x00866f04);
    GP4_GLOBAL(uint16_t, 0x00870608) = GP4_FIELD(uint16_t, second, 0x60);
    const uint16_t w62 = GP4_FIELD(uint16_t, second, 0x62);
    GP4_GLOBAL(uint16_t, 0x00870604) = w62;

    gp4::Regs d = {};
    d.eax = (ax_part & 0xffff0000u) | w62;
    d.ebx = bx_part;
    d.ecx = GP4_GLOBAL(uint8_t, 0x00852c38) != 0 ? GP4_GLOBAL(uint32_t, 0x00852c48) : 0x280u;
    d.edx = (c.edx << 16) | 8u;
    d.esi = c.esi;
    d.edi = reinterpret_cast<uint32_t>(second);
    d.ebp = r->ebp;
    d.eflags = c.eflags;
    gp4::call_regs(0x00559663, d);

    GP4_GLOBAL(uint16_t, 0x00874b68) = GP4_GLOBAL(uint16_t, 0x00934df0);
    GP4_GLOBAL(uint16_t, 0x00874b24) = static_cast<uint16_t>(d.eax);
    GP4_GLOBAL(uint16_t, 0x00874b26) = static_cast<uint16_t>(d.edx);
    uint16_t cx = static_cast<uint16_t>(~d.ecx);
    const uint16_t bx = static_cast<uint16_t>(0u - d.ebx);
    if (bx == 0) {
        cx = static_cast<uint16_t>(cx + 1u);
    }
    GP4_GLOBAL(uint16_t, 0x00874b28) = bx;
    GP4_GLOBAL(uint16_t, 0x00874b2a) = cx;

    const uint32_t width = GP4_GLOBAL(uint8_t, 0x00852c38) != 0 ? GP4_GLOBAL(uint32_t, 0x00852c44) : 0x280u;
    GP4_GLOBAL(uint16_t, 0x00870608) = static_cast<uint16_t>(width);
    GP4_GLOBAL(uint16_t, 0x00870604) = static_cast<uint16_t>(width);
    const uint16_t ax = static_cast<uint16_t>(width + 0x50u);
    GP4_GLOBAL(uint16_t, 0x0087060a) = ax;
    GP4_GLOBAL(uint16_t, 0x00870602) = 0;
    GP4_GLOBAL(uint16_t, 0x00870606) = 0;
    GP4_GLOBAL(uint16_t, 0x00870610) = 0;
    GP4_GLOBAL(uint16_t, 0x00870612) = 0;
    GP4_GLOBAL(uint16_t, 0x00870614) = 0;
    GP4_GLOBAL(uint16_t, 0x00870616) = 0;
    const uint8_t b48 = static_cast<uint8_t>(GP4_FIELD(uint8_t, second, 0x48) & 0xcfu);
    GP4_FIELD(uint8_t, second, 0x48) = b48;

    r->eax = (width & 0xffff0000u) | ax;
    r->ebx = (d.ebx & 0xffff0000u) | bx;
    r->ecx = (d.ecx & 0xffff0000u) | cx;
    r->edx = d.edx;
    r->esi = d.esi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(b48));
}
GP4_IMPL(0x005596fd, FUN_005596fd, "regs() -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=records_008943a0+hdg_00428216+ringf_005596fd")
