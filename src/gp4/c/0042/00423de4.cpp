#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_00621aa0 {
    int32_t UnknownX;
    int32_t UnknownY;
    int32_t UnknownZ;
    uint8_t Unknown0C[0x3c - 0x0c];
    int32_t Unknown3C;
    uint8_t* Body40;
    uint8_t Unknown44[0x50 - 0x44];
    int32_t Unknown50;
    uint8_t Unknown54[0x64 - 0x54];
};
static_assert(sizeof(Unknown_00621aa0) == 0x64, "record size");

// For each of the DAT_00609864 records (0x64 bytes) at 0x00621aa0: advances the value at +0x3c by
// (+0x50 * DAT_00607fa0) >> 11, averages old and new, scales that by DAT_00607fa0 >> 16 into
// DAT_00608298 and rotates (DAT_0060829c, DAT_0060829c, DAT_00608298) with 0x00401f02 through the
// body pointed to by +0x40; the rounded results are added to the record's x/y/z (+0/+4/+8).
// Leaves eax = the last DAT_00623e2c, ecx = the count, edx = the last body, ebp = the last old +0x3c;
// the flags are ZF|PF in every case (test of a zero count, or the final equal cmp).
// FUNCTION: GP4 0x00423de4
void __cdecl FUN_00423de4(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00609864);
    if (count != 0u) {
        Unknown_00621aa0* record = GP4_ARRAY(Unknown_00621aa0, 0x00621aa0);
        uint32_t index = 0;
        uint32_t old = 0;
        uint8_t* body = 0;
        do {
            old = static_cast<uint32_t>(record->Unknown3C);
            const int64_t step = static_cast<int64_t>(record->Unknown50) * GP4_GLOBAL(int32_t, 0x00607fa0);
            const uint32_t next = static_cast<uint32_t>(step >> 11) + old;
            record->Unknown3C = static_cast<int32_t>(next);
            const int64_t mean = static_cast<int64_t>(static_cast<int32_t>(next + old) >> 1) * GP4_GLOBAL(int32_t, 0x00607fa0);
            GP4_GLOBAL(uint32_t, 0x00608298) = static_cast<uint32_t>(mean >> 16);

            body = record->Body40;
            GP4_FIELD(double, body, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x0060829c));
            GP4_FIELD(double, body, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x0060829c));
            GP4_FIELD(double, body, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00608298));

            gp4::Regs call = *r;
            call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
            gp4::call_regs(0x00401f02, call);

            GP4_GLOBAL(int32_t, 0x00623e24) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa0));
            GP4_GLOBAL(int32_t, 0x00623e28) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa8));
            GP4_GLOBAL(int32_t, 0x00623e2c) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb0));

            record->UnknownX += GP4_GLOBAL(int32_t, 0x00623e24);
            record->UnknownY += GP4_GLOBAL(int32_t, 0x00623e28);
            record->UnknownZ += GP4_GLOBAL(int32_t, 0x00623e2c);
            ++record;
            ++index;
        } while (index != count);

        r->eax = GP4_GLOBAL(uint32_t, 0x00623e2c);
        r->ecx = index;
        r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
        r->ebp = old;
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00423de4, FUN_00423de4, "regs() -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00423de4")
