#include <gp4/gp4.h>

struct Unknown_004168fd {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0xce - 0x72];
    uint8_t FlagsCE;
    uint8_t UnknownCF[0xe4 - 0xcf];
    int8_t UnknownE4;
    uint8_t UnknownE5[0xff - 0xe5];
    uint8_t FlagsFF;
    uint8_t Unknown100[0x21d - 0x100];
    uint8_t Flags21D;
    uint8_t Unknown21E[0x26b - 0x21e];
    uint8_t Flags26B;
};

// esi = car record, ebp = the signed dword [0x00638ce0] (ecx = [0x00638cd8] is loaded as well).
// Byte +0x71 bit 2 mirrors "ebp < 0". Unless +0xff bit 0 is set: +0x21d bit 7 is raised when it is
// clear and either ebp >= 0x1d5, or (+0x26b bit 3 clear, signed byte +0xe4 outside 2..5 and either
// +0xce bit 6 or +0xff bit 3); ebp >= 0x75500 then sets +0xff bit 0 and +0x21d bit 7 as well.
// The routine ends in 0x0041ccd0, whose register results (eax, ebx, edx) and flags are the result.
// FUNCTION: GP4 0x004168fd
void __cdecl FUN_004168fd(gp4::Regs* r) {
    Unknown_004168fd* car = reinterpret_cast<Unknown_004168fd*>(static_cast<uintptr_t>(r->esi));

    r->ecx = GP4_GLOBAL(uint32_t, 0x00638cd8);
    r->ebp = GP4_GLOBAL(uint32_t, 0x00638ce0);
    const int32_t value = static_cast<int32_t>(r->ebp);

    car->Flags71 &= 0xfbu;
    if (value < 0) {
        car->Flags71 |= 4u;
    }

    if ((car->FlagsFF & 1u) == 0u) {
        if ((car->Flags21D & 0x80u) == 0u) {
            bool raise = false;
            if (value >= 0x1d5) {
                raise = true;
            } else if ((car->Flags26B & 8u) == 0u) {
                if (car->UnknownE4 < 2 || car->UnknownE4 > 5) {
                    raise = (car->FlagsCE & 0x40u) != 0u || (car->FlagsFF & 8u) != 0u;
                }
            }
            if (raise) {
                car->Flags21D |= 0x80u;
            }
        }
        if (value >= 0x75500) {
            car->FlagsFF |= 1u;
            car->Flags21D |= 0x80u;
        }
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0041ccd0, call);

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004168fd, FUN_004168fd, "regs(esi:ptr[0x273]:layout=car_004168fd) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+trig_00622e98+pos_0041ccd0")
