#include <gp4/gp4.h>
#include <gp4/flags.h>

// Cached write of ecx bytes from esi to the temp-file offset edi (cache buffer [0x829c44], its end
// [0x829c48], capacity check [0x829c4c], cached range [0x829c54]..[0x829c58] valid when the byte
// [0x829c50] is 0xff, dirty byte [0x829c51], file mode [0x829c68]; the flush is 0x0053859e).
// A count above [0x829c4c], an offset or end above 0x3b5380 or an overflowing end fails: the empty
// report routine 0x0054e232 runs with esi = 0x538421 and CF is set (other flags of the failing
// compare). With a valid cache, a request outside the cached range, past the cache end, or (edx = -1
// and a file mode set) leaving less than 0x1c200 bytes of room flushes it: with mode 0 the routine
// returns CF set; otherwise 0x0053859e writes [0x829c54]..[0x829c58] from the cache, the cache is
// invalidated and edx = 0, and a failed flush fails as above. Then a new cache range starts at edi
// (unless still valid), its end is raised to edi + ecx, the bytes are copied to [0x829c44] + (edi -
// [0x829c54]) by `rep movsb`, the dirty byte is set and CF is cleared (other flags of the final add).
// FUNCTION: GP4 0x0053832b
void __cdecl FUN_0053832b(gp4::Regs* r) {
    const uint32_t count = r->ecx;
    const uint32_t offset = r->edi;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t arith = 0;
    bool fail = false;

    arith = gp4::flags::sub32(count, GP4_GLOBAL(uint32_t, 0x00829c4c));
    if (count > GP4_GLOBAL(uint32_t, 0x00829c4c)) {
        fail = true;
    } else {
        arith = gp4::flags::sub32(offset, 0x3b5380u);
        if (offset > 0x3b5380u) {
            fail = true;
        } else {
            eax = offset + count;
            arith = gp4::flags::add32(offset, count);
            if (eax < offset) {
                fail = true;
            } else {
                arith = gp4::flags::sub32(eax, 0x3b5380u);
                if (eax > 0x3b5380u) {
                    fail = true;
                }
            }
        }
    }

    if (!fail && GP4_GLOBAL(uint8_t, 0x00829c50) == 0xffu) {
        bool flush = false;
        if (offset < GP4_GLOBAL(uint32_t, 0x00829c54)) {
            flush = true;
        } else if (offset > GP4_GLOBAL(uint32_t, 0x00829c58)) {
            flush = true;
        } else {
            eax = offset - GP4_GLOBAL(uint32_t, 0x00829c54) + GP4_GLOBAL(uint32_t, 0x00829c44) + count;
            if (eax > GP4_GLOBAL(uint32_t, 0x00829c48)) {
                flush = true;
            } else if (edx == 0xffffffffu && GP4_GLOBAL(uint8_t, 0x00829c68) != 0u) {
                eax += 0x1c200u;
                if (eax >= GP4_GLOBAL(uint32_t, 0x00829c48)) {
                    flush = true;
                }
            }
        }
        if (flush) {
            const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00829c68);
            if (mode == 0u) {
                r->eax = eax;
                r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0u) | gp4::flags::CF);
                return;
            }
            gp4::Regs c = *r;
            c.eax = eax;
            c.esi = GP4_GLOBAL(uint32_t, 0x00829c44);
            c.edi = GP4_GLOBAL(uint32_t, 0x00829c54);
            c.ecx = GP4_GLOBAL(uint32_t, 0x00829c58) - c.edi;
            gp4::call_regs(0x0053859e, c);
            GP4_GLOBAL(uint8_t, 0x00829c50) = 0u;
            edx = 0u;
            if ((c.eflags & gp4::flags::CF) != 0u) {
                arith = c.eflags;
                fail = true;
            }
        }
    }

    if (fail) {
        gp4::Regs c = *r;
        c.eax = eax;
        c.edx = edx;
        c.esi = 0x00538421u;
        c.eflags = gp4::flags::merge(r->eflags, arith);
        gp4::call_regs(0x0054e232, c);
        r->eax = eax;
        r->edx = edx;
        r->eflags = gp4::flags::merge(r->eflags, c.eflags | gp4::flags::CF);
        return;
    }

    if (GP4_GLOBAL(uint8_t, 0x00829c50) != 0xffu) {
        GP4_GLOBAL(uint32_t, 0x00829c54) = offset;
        GP4_GLOBAL(uint32_t, 0x00829c58) = offset;
        GP4_GLOBAL(uint8_t, 0x00829c50) = 0xffu;
    }
    eax = offset + count;
    if (eax > GP4_GLOBAL(uint32_t, 0x00829c58)) {
        GP4_GLOBAL(uint32_t, 0x00829c58) = eax;
    }
    const uint32_t relative = offset - GP4_GLOBAL(uint32_t, 0x00829c54);
    const uint32_t destination = relative + GP4_GLOBAL(uint32_t, 0x00829c44);
    arith = gp4::flags::add32(relative, GP4_GLOBAL(uint32_t, 0x00829c44));
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(destination));
    for (uint32_t k = count; k != 0u; --k) {
        *dst++ = *src++;
    }
    GP4_GLOBAL(uint8_t, 0x00829c51) = 0xffu;
    r->eax = eax;
    r->ecx = 0u;
    r->edx = edx;
    r->esi += count;
    r->edi = destination + count;
    r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::flags::CF);
}
GP4_IMPL(0x0053832b, FUN_0053832b, "regs(eax:u32, ecx:u32[0..0x200], edx:u32{0xffffffff,0,1}, esi:ptr[0x200]:bytes, edi:u32[0..0x400]) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=cache_0053832b")
