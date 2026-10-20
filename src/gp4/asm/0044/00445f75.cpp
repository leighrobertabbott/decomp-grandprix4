#include <gp4/gp4.h>

// pushal / popal wrapper: for every set bit n (0..31) of the mask in eax, retires the car whose slot is bound
// to n. 0x00445a0a looks the key n up in the 22-byte slot table at 0x0062f916 (CF clear = found, eax = slot);
// the slot is freed (byte set to 0xff), the car number [slot + 0x007c2a48] is looked up among the 22 car
// records through 0x004459e5 (CF clear = found, eax = record), the record's byte +0x20c is set to 0xff and the
// car is retired through 0x00445fb4 (esi = record). Registers are restored; the flags are those of the final
// `cmp ecx, 0x20` (equal: ZF and PF).
// FUNCTION: GP4 0x00445f75
void __cdecl FUN_00445f75(gp4::Regs* r) {
    uint32_t mask = r->eax;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t bit = 0u;
    do {
        const bool take = (mask & 1u) != 0u;
        mask >>= 1;
        c.ecx = bit;
        if (take) {
            c.eax = (c.eax & 0xffffff00u) | bit;
            gp4::call_regs(0x00445a0a, c);
            if ((c.eflags & 1u) == 0u) {
                const uint32_t slot = c.eax;
                GP4_ARRAY(uint8_t, 0x0062f916)[slot] = 0xff;
                c.eax = (c.eax & 0xffffff00u) | GP4_ARRAY(const uint8_t, 0x007c2a48)[slot];
                gp4::call_regs(0x004459e5, c);
                if ((c.eflags & 1u) == 0u) {
                    c.esi = c.eax;
                    GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi)), 0x20c) = 0xff;
                    gp4::call_regs(0x00445fb4, c);
                }
            }
        }
        ++bit;
    } while (bit != 0x20u);

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00445f75, FUN_00445f75, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0041104f+flag_007a97a0+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab590+carst_0040f6d0+carno_0066ff6c+state_00434602+state_00412b88+slots_0062f916+selcar_0062f8b0")
