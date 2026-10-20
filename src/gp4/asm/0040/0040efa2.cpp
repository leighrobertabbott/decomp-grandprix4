#include <gp4/gp4.h>

// Car block behind esi (only the fields this routine reads / writes).
struct Unknown_0040efa2_Car {
    uint8_t Unknown00[0x20];
    uint32_t Unknown20;
    uint32_t Unknown24;
    uint32_t Unknown28;
    uint32_t Unknown2C;
    uint32_t Unknown30;
    uint32_t Unknown34;
    uint32_t Unknown38;
    uint8_t Unknown3C[0x88 - 0x3c];
    uint8_t Flags88;
    uint8_t Unknown89[0xc0 - 0x89];
    uint32_t UnknownC0;
    uint8_t UnknownC4[0x162 - 0xc4];
    uint8_t Flags162;
};

// esi = car block, edi = its segment record. With bit 4 of +0x162 set only 0x0040ec92 runs. Otherwise
// 0x0040e9ac gives ebx (the lap-time fraction), 0x0040ed6a records it, and +0xc0 becomes
// [0x0066d814] - ebx: the first time (+0xc0 was 0xc0000000) nothing else happens; later the change
// goes to +0x28, 0x0040edc3 logs it, +0x38 takes the previous +0x34 and, when the change is below
// +0x34, that becomes the new +0x34 with +0x20 / +0x24 copied to +0x2c / +0x30 and
// 0x0040f06c, 0x0040ee7b run (and bit 1 of +0x88 is set unless byte 0x007c2ea9 has its top bit set).
// 0x0040f3f6 finishes every path but the first. All registers and the flags are what the last callee leaves.
// FUNCTION: GP4 0x0040efa2
void __cdecl FUN_0040efa2(gp4::Regs* r) {
    Unknown_0040efa2_Car* self = reinterpret_cast<Unknown_0040efa2_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    if ((self->Flags162 & 0x10u) != 0u) {
        gp4::call_regs(0x0040ec92, s);
    } else {
        gp4::call_regs(0x0040e9ac, s);
        gp4::call_regs(0x0040ed6a, s);
        if (self->UnknownC0 == 0xc0000000u) {
            s.eax = GP4_GLOBAL(uint32_t, 0x0066d814) - s.ebx;
            self->UnknownC0 = s.eax;
        } else {
            s.edx = self->UnknownC0;
            s.eax = GP4_GLOBAL(uint32_t, 0x0066d814) - s.ebx;
            self->UnknownC0 = s.eax;
            s.eax -= s.edx;
            self->Unknown28 = s.eax;
            gp4::call_regs(0x0040edc3, s);
            self->Unknown38 = self->Unknown34;
            if (static_cast<int32_t>(s.eax) < static_cast<int32_t>(self->Unknown34)) {
                self->Unknown34 = s.eax;
                s.eax = self->Unknown20;
                self->Unknown2C = s.eax;
                s.eax = self->Unknown24;
                self->Unknown30 = s.eax;
                gp4::call_regs(0x0040f06c, s);
                gp4::call_regs(0x0040ee7b, s);
                if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x007c2ea9)) >= 0) {
                    self->Flags88 = static_cast<uint8_t>(self->Flags88 | 0x02u);
                }
            }
        }
        gp4::call_regs(0x0040f3f6, s);
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x0040efa2, FUN_0040efa2, "regs(eax:u32, ecx:u32, edx:u32, edi:ptr[0x40]:bytes, esi:ptr[0x420]:layout=car_004129fa+rec_0040e9ac+rec_0040ee7b+rec_0040efa2) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040e9ac+state_0040edc3+state_0040ee7b+carno_0066ff6c+order_00675958+state_0040f06c+state_0040efa2")
