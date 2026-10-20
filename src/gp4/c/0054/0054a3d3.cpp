#include <gp4/gp4.h>
#include <gp4/flags.h>

// Variable-index table accesses of the command tables (26 expected-length bytes at 0x83327d and 27 handler
// addresses at 0x833211).
static __declspec(noinline) uint32_t expectedLengthSlot(uint32_t index) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const uint8_t, 0x0083327d) + index));
}

static __declspec(noinline) uint32_t handlerSlot(uint32_t index) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const uint32_t, 0x00833211) + index));
}

// The rejection tail: the empty stub 0x0054e232 runs inside pushal/popal (so the registers stay as they were),
// then stc.
static void rejectPacket(gp4::Regs* r, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t arith) {
    gp4::cdecl_<0x0054e232, void>();
    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
}

// FUNCTION: GP4 0x0054a3d3
// Network packet dispatcher: [0x7a97ac] is the packet length (1..0x100) and the byte at 0x7a97b1 the command (bit
// 0x80 sets the flag byte [0x7aafd9] to 0xff, the low 5 bits select the command). Commands 0x1a..0x1d share
// handler 0x1a; the others must have the packet length (low byte) listed in the table at 0x83327d. A good
// packet is handled by the routine at 0x833211[command] (registers and flags come back from it); a bad length, a
// command above 0x1d or a length mismatch runs the empty stub 0x0054e232 and returns with CF set.
void __cdecl FUN_0054a3d3(gp4::Regs* r) {
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x007a97ac);
    uint32_t arith = gp4::flags::sub32(length, 1u);
    if (length < 1u) {
        rejectPacket(r, r->eax, r->ebx, length, arith);
        return;
    }
    arith = gp4::flags::sub32(length, 0x100u);
    if (length > 0x100u) {
        rejectPacket(r, r->eax, r->ebx, length, arith);
        return;
    }
    if ((GP4_GLOBAL(uint8_t, 0x007a97b1) & 0x80u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x007aafd9) = 0xffu;
    }
    uint32_t command = GP4_GLOBAL(uint8_t, 0x007a97b1) & 0x1fu;
    arith = gp4::flags::sub32(command, 0x1du);
    if (command > 0x1du) {
        rejectPacket(r, command, r->ebx, length, arith);
        return;
    }
    arith = gp4::flags::sub32(command, 0x1au);
    if (command < 0x1au) {
        const uint32_t slot = expectedLengthSlot(command);
        const uint32_t expected = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(slot));
        arith = gp4::flags::sub8(length, expected);
        if ((length & 0xffu) != expected) {
            rejectPacket(r, command, slot, length, arith);
            return;
        }
    } else {
        command = 0x1au;
    }
    const uint32_t handlerEntry = handlerSlot(command);
    arith = gp4::flags::add32(command << 2, 0x00833211u);

    gp4::Regs c = *r;
    c.eax = command;
    c.ebx = handlerEntry;
    c.ecx = length;
    c.eflags = gp4::flags::merge(r->eflags, arith);
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(handlerEntry)), c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054a3d3, FUN_0054a3d3, "regs(eax:u32, ebx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=ctr_0054a678+pkt_0054a3d3")
