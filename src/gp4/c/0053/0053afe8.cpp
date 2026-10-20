#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053afe8
// Queues the sort-list node for the record esi: 0x00531af4 stores 0 into [0x827559], then ebx, eax, ebp and
// ecx are loaded from the words at esi+4, +6, +8 and +0xa (zero-extended) and 0x00549db9 inserts the node
// {ebx, eax = sort key, ebp, ecx} into the pool list; 0x00531b2e is an empty routine. The four words stay in
// ebx, eax, ebp and ecx; the flags are those 0x00549db9 leaves.
void __cdecl FUN_0053afe8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0u;
    gp4::call_regs(0x00531af4, c);

    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    c.ebx = GP4_FIELD(uint16_t, rec, 4);
    c.eax = GP4_FIELD(uint16_t, rec, 6);
    c.ebp = GP4_FIELD(uint16_t, rec, 8);
    c.ecx = GP4_FIELD(uint16_t, rec, 0xa);
    gp4::call_regs(0x00549db9, c);
    gp4::cdecl_<0x00531b2e, void>();

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ebp = c.ebp;
    r->ecx = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053afe8, FUN_0053afe8, "regs(esi:ptr[0x10]:bytes) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")
