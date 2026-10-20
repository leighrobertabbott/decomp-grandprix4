#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00559be4
// For the five dword attribute fields +0x30, +0x34, +0x38, +0x40, +0x3c (in that order): turns bit
// 0x400 of the source record's (eax) field into the sign bit (via 0x00559bd0) and ORs it into the
// same field of the destination record (edi). edx is saved and restored; the flags are those of
// the final `or dword [edi+0x3c], edx`.
static void MergeAttribute(const uint8_t* src, uint8_t* dst, uint32_t off, uint32_t* value) {
    gp4::Regs call = {};
    call.edx = 0x400u & *reinterpret_cast<const uint32_t*>(src + off);
    gp4::call_regs(0x00559bd0, call);
    *value = *reinterpret_cast<uint32_t*>(dst + off) | call.edx;
    *reinterpret_cast<uint32_t*>(dst + off) = *value;
}

void __cdecl FUN_00559be4(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->eax));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t value;
    MergeAttribute(src, dst, 0x30u, &value);
    MergeAttribute(src, dst, 0x34u, &value);
    MergeAttribute(src, dst, 0x38u, &value);
    MergeAttribute(src, dst, 0x40u, &value);
    MergeAttribute(src, dst, 0x3cu, &value);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x00559be4, FUN_00559be4, "regs(eax:ptr[0x48]:u32{0,0x400,0x80000000,0x400,0x1,0xffffffff,0x12345678}, edi:ptr[0x48]:u32{0,0,0x80000000,0x1,0xffffffff,0x12345678}) -> (cf, pf, af, zf, sf, of)")
