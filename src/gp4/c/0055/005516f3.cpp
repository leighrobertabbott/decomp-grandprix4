#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills two consecutive 5-record tables (stride 0x14, at 0x009654ac and 0x00965510) with the same
// pair of dwords per table: records of the first get [0x0096548c] at +0xc and [0x00965490] at +0x10,
// records of the second get [0x00965494] / [0x00965498]. Ends with edi one past the second table
// (0x00965574), ecx = 0, eax = the last value stored ([0x00965498]); the flags are those of the final
// "add edi, 0x14" (loop leaves them alone), which are constant.
// FUNCTION: GP4 0x005516f3
void __cdecl FUN_005516f3(gp4::Regs* r) {
    uint32_t edi = 0x009654acu;
    uint32_t eax = 0u;
    uint32_t arith = 0u;
    for (uint32_t n = 0u; n < 5u; ++n) {
        uint8_t* rec = GP4_ARRAY(uint8_t, 0x009654ac) + (edi - 0x009654acu);
        *(uint32_t*)(rec + 0xc) = GP4_GLOBAL(uint32_t, 0x0096548c);
        eax = GP4_GLOBAL(uint32_t, 0x00965490);
        *(uint32_t*)(rec + 0x10) = eax;
        arith = gp4::flags::add32(edi, 0x14u);
        edi += 0x14u;
    }
    edi = 0x00965510u;
    for (uint32_t n = 0u; n < 5u; ++n) {
        uint8_t* rec = GP4_ARRAY(uint8_t, 0x00965510) + (edi - 0x00965510u);
        *(uint32_t*)(rec + 0xc) = GP4_GLOBAL(uint32_t, 0x00965494);
        eax = GP4_GLOBAL(uint32_t, 0x00965498);
        *(uint32_t*)(rec + 0x10) = eax;
        arith = gp4::flags::add32(edi, 0x14u);
        edi += 0x14u;
    }
    r->eax = eax;
    r->ecx = 0u;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005516f3, FUN_005516f3, "regs() -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
