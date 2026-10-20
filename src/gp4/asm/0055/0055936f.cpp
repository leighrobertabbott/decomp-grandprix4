#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fixed-point division: the dividend is the signed byte bl shifted left by 11, the divisor the
// signed word at +0x62 of the edi record (1 when it is 0). Returns eax = the divisor and ebx = the
// quotient; edx is saved and restored (cdq/idiv use it). The flags are those of
// `test ebx, ebx` on the sign-extended word (idiv leaves them alone; the `mov ebx, 1` fix-up does
// not touch them either).
// FUNCTION: GP4 0x0055936f
void __cdecl FUN_0055936f(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    int32_t divisor = static_cast<int16_t>(GP4_FIELD(uint16_t, record, 0x62));
    const int32_t dividend = static_cast<int32_t>(static_cast<int8_t>(r->ebx)) << 11;
    const uint32_t arith = gp4::flags::logic32(static_cast<uint32_t>(divisor));
    if (divisor == 0) {
        divisor = 1;
    }
    r->ebx = static_cast<uint32_t>(dividend / divisor);
    r->eax = static_cast<uint32_t>(divisor);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055936f, FUN_0055936f, "regs(ebx:u32, edi:ptr[0x6a]:u8{=0,0,0,1,0x7f,0x80,0xff}) -> (eax, ebx, cf, pf, af, zf, sf, of)")
