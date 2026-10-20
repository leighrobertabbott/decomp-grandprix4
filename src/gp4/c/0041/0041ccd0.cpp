#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_006380d8 {
    uint8_t Unknown00[0x4c];
    int32_t Unknown4C;
    int32_t Unknown50;
    int32_t Unknown54;
    uint8_t Unknown58[0x190 - 0x58];
    int32_t Unknown190;
    int32_t Unknown194;
    uint8_t Unknown198[0x300 - 0x198];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record size");

// Rotates the integer pair (DAT_00623eb8, DAT_00623ec0) and the pair (DAT_0063804c >> 8,
// DAT_00638054 >> 8) by the angle held in the body at 0x00622e98 (sin at +0x38, cos at +0x40; the two
// rotated pairs go to DAT_00622bc0/bc8 and DAT_00622bb0/bb8). Then, for each of the four 0x300-byte
// records at 0x006380d8, the integers +0x4c/+0x50/+0x54 become doubles at +0xb8/+0xc0/+0xc8 of the
// body, 0x00401f02 rotates them (results at +0xa0/+0xa8/+0xb0) and two combinations of those with
// DAT_00623ebc, the rotated pairs and the scale DAT_006f2bcc are rounded (FISTP) into the record's
// +0x190 and +0x194. Every sum stays on the x87 stack (80-bit) until the store.
// Leaves eax = DAT_0063804c >> 8, ebx = 0x00638cd8, edx = the body and the flags of the final
// "cmp ebx, 0x638cd8" (equal: ZF and PF set).
// FUNCTION: GP4 0x0041ccd0
void __cdecl FUN_0041ccd0(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622e98);

    const Ext inA = ext_i32(GP4_GLOBAL(int32_t, 0x00623eb8));
    const Ext inB = ext_i32(GP4_GLOBAL(int32_t, 0x00623ec0));
    GP4_GLOBAL(double, 0x00622bc0) =
        narrow(inA * ext(GP4_FIELD(double, body, 0x40)) - inB * ext(GP4_FIELD(double, body, 0x38)));
    GP4_GLOBAL(double, 0x00622bc8) =
        narrow(inA * ext(GP4_FIELD(double, body, 0x38)) + inB * ext(GP4_FIELD(double, body, 0x40)));

    GP4_GLOBAL(int32_t, 0x006f2bf8) = GP4_GLOBAL(int32_t, 0x00638054) >> 8;
    const Ext inC = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf8));
    GP4_GLOBAL(int32_t, 0x006f2bf8) = GP4_GLOBAL(int32_t, 0x0063804c) >> 8;
    const Ext inD = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf8));
    GP4_GLOBAL(double, 0x00622bb0) =
        narrow(inD * ext(GP4_FIELD(double, body, 0x40)) - inC * ext(GP4_FIELD(double, body, 0x38)));
    GP4_GLOBAL(double, 0x00622bb8) =
        narrow(inD * ext(GP4_FIELD(double, body, 0x38)) + inC * ext(GP4_FIELD(double, body, 0x40)));

    Unknown_006380d8* record = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    do {
        GP4_FIELD(double, body, 0xb8) = static_cast<double>(record->Unknown4C);
        GP4_FIELD(double, body, 0xc0) = static_cast<double>(record->Unknown50);
        GP4_FIELD(double, body, 0xc8) = static_cast<double>(record->Unknown54);

        gp4::Regs call = *r;
        call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
        gp4::call_regs(0x00401f02, call);

        record->Unknown190 = fistp32(
            (ext(GP4_GLOBAL(double, 0x00622bc8)) * ext(GP4_FIELD(double, body, 0xa8)) -
             ext_i32(GP4_GLOBAL(int32_t, 0x00623ebc)) * ext(GP4_FIELD(double, body, 0xb0))) *
                ext(GP4_GLOBAL(double, 0x006f2bcc)) +
            ext(GP4_GLOBAL(double, 0x00622bb0)));
        record->Unknown194 = fistp32(
            (ext_i32(GP4_GLOBAL(int32_t, 0x00623ebc)) * ext(GP4_FIELD(double, body, 0xa0)) -
             ext(GP4_GLOBAL(double, 0x00622bc0)) * ext(GP4_FIELD(double, body, 0xa8))) *
                ext(GP4_GLOBAL(double, 0x006f2bcc)) +
            ext(GP4_GLOBAL(double, 0x00622bb8)));
        ++record;
    } while (reinterpret_cast<uintptr_t>(record) != 0x00638cd8u);

    r->eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006f2bf8));
    r->ebx = 0x00638cd8u;
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    // cmp ebx, 0x638cd8 with equal operands: ZF and PF set, everything else clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041ccd0, FUN_0041ccd0, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+trig_00622e98+pos_0041ccd0")
