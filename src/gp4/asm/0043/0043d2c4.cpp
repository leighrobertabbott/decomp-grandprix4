#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043d2c4
// Length of a 2D vector held as two (byte + word>>6) pairs in the record at edi:
//   a = (int8)[edi+0x11] + ([edi+0x0c] >> 6)      (16-bit arithmetic shift)
//   b = (int8)[edi+0x13] + ([edi+0x0e] >> 6)
// dx:ax = a*a + b*b (signed 16x16 products, summed with carry) goes to the integer square
// root 0x409c44 and its ax result is stored to the word at 0x6077da. eax/ebx/ecx/edx are
// saved and restored, so the flags (left by the square root) are the only other output.
void __cdecl FUN_0043d2c4(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int16_t a = static_cast<int16_t>(
        static_cast<int16_t>(static_cast<int8_t>(GP4_FIELD(uint8_t, rec, 0x11))) +
        static_cast<int16_t>(GP4_FIELD(int16_t, rec, 0x0c) >> 6));
    const int16_t b = static_cast<int16_t>(
        static_cast<int16_t>(static_cast<int8_t>(GP4_FIELD(uint8_t, rec, 0x13))) +
        static_cast<int16_t>(GP4_FIELD(int16_t, rec, 0x0e) >> 6));

    const uint32_t sumSquares = static_cast<uint32_t>(static_cast<int32_t>(a) * static_cast<int32_t>(a)) +
                                static_cast<uint32_t>(static_cast<int32_t>(b) * static_cast<int32_t>(b));

    gp4::Regs call = *r;
    call.eax = (r->eax & 0xffff0000u) | (sumSquares & 0xffffu);
    call.edx = sumSquares >> 16;
    gp4::call_regs(0x00409c44, call);

    GP4_GLOBAL(uint16_t, 0x006077da) = static_cast<uint16_t>(call.eax);

    // pop edx / ecx / ebx / eax restore the entry values; every flag comes from the callee.
    call.eax = r->eax;
    call.ebx = r->ebx;
    call.ecx = r->ecx;
    call.edx = r->edx;
    *r = call;
}
GP4_IMPL(0x0043d2c4, FUN_0043d2c4, "regs(edi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
