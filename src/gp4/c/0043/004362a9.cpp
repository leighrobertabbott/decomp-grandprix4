#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the car-order list 0x0066fe3c (words = byte offsets of the car records at 0x0066fef0)
// up to the end pointer [0x006043f8] and runs the per-car step 0x0040f1d1 for every listed car:
// esi = the car record, edi = its current segment record (car +0x1c), ebx = the next list slot.
// ebx is saved around each call (push/pop), so the walk continues from the slot; every other
// register is threaded from one call to the next. The flags left are those of the final
// `cmp ebx, [0x006043f8]`.
// FUNCTION: GP4 0x004362a9
void __cdecl FUN_004362a9(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    const int16_t* entry = GP4_ARRAY(int16_t, 0x0066fe3c);
    uint8_t* const cars = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t slot;
    for (;;) {
        uint8_t* car = cars + *entry;
        s.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
        s.edi = GP4_FIELD(uint32_t, car, 0x1c);
        ++entry;
        slot = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
        s.ebx = slot;
        gp4::call_regs(0x0040f1d1, s);
        s.ebx = slot;
        if (slot == GP4_GLOBAL(uint32_t, 0x006043f8)) {
            break;
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = gp4::flags::merge(s.eflags, gp4::flags::sub32(slot, slot));
}
GP4_IMPL(0x004362a9, FUN_004362a9, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+state_0040e9ac+state_0040edc3+state_0040ee7b+carno_0066ff6c+order_00675958+state_0040f06c+state_0040efa2+state_0040e949+state_0040e84e+state_004327cd+state_0040eb6b+state_0040ed29+state_0040f112+counter_0040f0dd+ring_008943a0+words_008943a0+ptr_0086850c+geom_008943a0+cars_0066fef0+carst_0040f6d0+state_0040e84e+list_0066fe3c+order_0042e97e+cursor_006043fc+cars_004362a9")
