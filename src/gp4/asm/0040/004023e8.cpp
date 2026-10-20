#include <gp4/gp4.h>
#include <gp4/x87.h>

// Builds the camera/body input vectors from the record at esi and 0x00602fcc..: the three dwords at 0x602fcc..
// become doubles (0x609760..), the words at esi+0x204/0x206/0x208 shifted left by 8 go to 0x623e18..0x623e20 and
// become doubles (0x609790..). 0x004024b5 derives and clamps the three components 0x00609778.., which are copied to
// the body at 0x00622e98 (+0xb8/+0xc0/+0xc8), 0x00401f02 rotates them into +0xa0/+0xa8/+0xb0 and those are
// rounded (FISTP) into 0x00601d44... eax = the last shifted word, edx = the body address, flags as 0x004024b5
// left them (OF as the final SHL left it).
// FUNCTION: GP4 0x004023e8
void __cdecl FUN_004023e8(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(double, 0x00609760) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00602fcc));
    GP4_GLOBAL(double, 0x00609768) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00602fd0));
    GP4_GLOBAL(double, 0x00609770) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00602fd4));

    GP4_GLOBAL(uint32_t, 0x00623e18) = static_cast<uint32_t>(GP4_FIELD(const uint16_t, record, 0x204)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e1c) = static_cast<uint32_t>(GP4_FIELD(const uint16_t, record, 0x206)) << 8;
    const uint32_t last = static_cast<uint32_t>(GP4_FIELD(const uint16_t, record, 0x208)) << 8;
    GP4_GLOBAL(uint32_t, 0x00623e20) = last;

    GP4_GLOBAL(double, 0x00609790) = static_cast<double>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00623e18)));
    GP4_GLOBAL(double, 0x00609798) = static_cast<double>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00623e1c)));
    GP4_GLOBAL(double, 0x006097a0) = static_cast<double>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00623e20)));

    r->eax = last;
    r->eflags &= ~0x800u;  // the final SHL eax, 8 of a zero-extended word leaves OF = 0
    gp4::call_regs(0x004024b5, *r);

    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622e98);
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    GP4_FIELD(double, body, 0xb8) = GP4_GLOBAL(double, 0x00609778);
    GP4_FIELD(double, body, 0xc0) = GP4_GLOBAL(double, 0x00609780);
    GP4_FIELD(double, body, 0xc8) = GP4_GLOBAL(double, 0x00609788);
    gp4::call_regs(0x00401f02, *r);
    GP4_GLOBAL(int32_t, 0x00601d44) = fistp32(GP4_FIELD(double, body, 0xa0));
    GP4_GLOBAL(int32_t, 0x00601d48) = fistp32(GP4_FIELD(double, body, 0xa8));
    GP4_GLOBAL(int32_t, 0x00601d4c) = fistp32(GP4_FIELD(double, body, 0xb0));
}
GP4_IMPL(0x004023e8, FUN_004023e8, "regs(esi:ptr[0x210]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
