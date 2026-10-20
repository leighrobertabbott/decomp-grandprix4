#include <gp4/gp4.h>
#include <gp4/flags.h>

// Side ebx (0 or 1) of the two-entry dword arrays at 0x852c8c.. (index ebx * 4).
static inline uint32_t& Side(uint32_t base, uint32_t side) {
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(base + side * 4u));
}

// from + ((to - from) * t >> 14): the 64-bit imul, shrd by 14 and add of the original.
static inline uint32_t Lerp(uint32_t to, uint32_t from, uint32_t t) {
    const int64_t prod = static_cast<int64_t>(static_cast<int32_t>(to - from)) * static_cast<int32_t>(t);
    return static_cast<uint32_t>(prod >> 14) + from;
}

// Interpolates side ebx across the [0x852d20] records at [0x852ccc] (stride 0x160): the end point
// (edx, eax, ecx) = arrays 0x852c8c/0x852c94/0x852c9c is rotated through 0x0055669e with the record
// [0x852cd4] into 0x852cf4/0x852cfc/0x852d04, the start point 0x852cac/0x852cb4/0x852cbc with the first
// record into 0x852cdc/0x852ce4/0x852cec. For record k (counter 0x852d24) t = (k << 14) / count, the
// four lerps of (0x852cc4 -> 0x852ca4), (cdc -> cf4), (ce4 -> cfc), (cec -> d04) are measured with
// 0x005565f3 and stored at +0x100/+0x104/+0x108/+0x10a (ebx = 0) or +0x10c/+0x110/+0x114/+0x116: the
// distance, the bearing, the callee's ecx >> 6 and the first lerp >> 6 (sar). Finally the end point
// becomes the start point (0x852cac.. = 0x852c8c.., 0x852cc4 = 0x852ca4).
// FUNCTION: GP4 0x00556791
void __cdecl FUN_00556791(gp4::Regs* r) {
    const uint32_t side = r->ebx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.edi = GP4_GLOBAL(uint32_t, 0x00852cd4);
    c.edx = Side(0x00852c8cu, side);
    c.ecx = Side(0x00852c9cu, side);
    c.eax = Side(0x00852c94u, side);
    gp4::call_regs(0x0055669e, c);
    Side(0x00852cf4u, side) = c.eax;
    Side(0x00852cfcu, side) = c.ecx;
    Side(0x00852d04u, side) = c.edx;

    c.edi = GP4_GLOBAL(uint32_t, 0x00852ccc);
    c.edx = Side(0x00852cacu, side);
    c.ecx = Side(0x00852cbcu, side);
    c.eax = Side(0x00852cb4u, side);
    gp4::call_regs(0x0055669e, c);
    Side(0x00852cdcu, side) = c.eax;
    Side(0x00852ce4u, side) = c.ecx;
    Side(0x00852cecu, side) = c.edx;

    GP4_GLOBAL(uint32_t, 0x00852d24) = 0u;
    uint32_t record = GP4_GLOBAL(uint32_t, 0x00852ccc);
    uint32_t count;
    do {
        int32_t rem;
        const uint32_t t = static_cast<uint32_t>(gp4::idiv64(
            static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x00852d24) << 14),
            GP4_GLOBAL(int32_t, 0x00852d20), &rem));
        const uint32_t p1 = Lerp(Side(0x00852ca4u, side), Side(0x00852cc4u, side), t);
        const uint32_t p2 = Lerp(Side(0x00852cf4u, side), Side(0x00852cdcu, side), t);
        const uint32_t p3 = Lerp(Side(0x00852cfcu, side), Side(0x00852ce4u, side), t);
        const uint32_t p4 = Lerp(Side(0x00852d04u, side), Side(0x00852cecu, side), t);

        c.edi = record;
        c.ebp = t;
        c.eax = p2;
        c.ecx = p3;
        c.edx = p4;
        gp4::call_regs(0x005565f3, c);
        c.ebp = p1;
        c.ecx = static_cast<uint32_t>(static_cast<int32_t>(c.ecx) >> 6);
        c.ebp = static_cast<uint32_t>(static_cast<int32_t>(c.ebp) >> 6);

        uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        if (side == 0u) {
            GP4_FIELD(uint32_t, rec, 0x104) = c.eax;
            GP4_FIELD(uint32_t, rec, 0x100) = c.edx;
            GP4_FIELD(uint16_t, rec, 0x108) = static_cast<uint16_t>(c.ecx);
            GP4_FIELD(uint16_t, rec, 0x10a) = static_cast<uint16_t>(c.ebp);
        } else {
            GP4_FIELD(uint32_t, rec, 0x110) = c.eax;
            GP4_FIELD(uint32_t, rec, 0x10c) = c.edx;
            GP4_FIELD(uint16_t, rec, 0x114) = static_cast<uint16_t>(c.ecx);
            GP4_FIELD(uint16_t, rec, 0x116) = static_cast<uint16_t>(c.ebp);
        }
        GP4_GLOBAL(uint32_t, 0x00852d24) += 1u;
        record += 0x160u;
        count = GP4_GLOBAL(uint32_t, 0x00852d24);
    } while (count != GP4_GLOBAL(uint32_t, 0x00852d20));
    const uint32_t arith = gp4::flags::sub32(count, GP4_GLOBAL(uint32_t, 0x00852d20));

    Side(0x00852cb4u, side) = Side(0x00852c94u, side);
    Side(0x00852cacu, side) = Side(0x00852c8cu, side);
    Side(0x00852cbcu, side) = Side(0x00852c9cu, side);
    const uint32_t last = Side(0x00852ca4u, side);
    Side(0x00852cc4u, side) = last;

    r->eax = last;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = record;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00556791, FUN_00556791, "regs(ebx:u32{=0,1}, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=multiply_0040ce7b+lerp_00556791")
