#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatches a HUD/option text record at esi on its first character: 'h' goes to the four-number
// parser 0x00514848 (tyre light records), every other character ('l' included) to the seven-byte
// record parser 0x00514821. The callee's registers and flags are the result.
static __declspec(noinline) void parse_record(gp4::Regs& c) {
    gp4::call_regs(0x00514821, c);
}

static __declspec(noinline) void parse_tyre_record(gp4::Regs& c) {
    gp4::call_regs(0x00514848, c);
}

// FUNCTION: GP4 0x00514877
void __cdecl FUN_00514877(gp4::Regs* r) {
    const uint8_t first = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    if (first == 0x68u) {
        parse_tyre_record(c);
    } else {
        parse_record(c);
    }
    *r = c;
}
GP4_IMPL(0x00514877, FUN_00514877, "regs(eax:u32, esi:ptr[0x100]:layout=text_00514877, edi:ptr[0x20]:bytes) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
