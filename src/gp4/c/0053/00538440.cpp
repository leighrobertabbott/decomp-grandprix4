#include <gp4/gp4.h>
#include <gp4/flags.h>

// Cached read of ecx bytes at the temp-file offset esi into edi (read twin of 0x0053832b: cache
// buffer [0x829c44], capacity [0x829c4c], cached range [0x829c54]..[0x829c58] valid when the byte
// [0x829c50] is 0xff, dirty byte [0x829c51], file mode [0x829c68]). A count above [0x829c4c], an
// offset or end above 0x3b5380 or an overflowing end fails: the empty report routine 0x0054e232
// runs with esi = 0x538540 and CF is set (other flags of the failing compare). A request outside
// the valid cached range, or (edx = -1 and a file mode set) ending less than 0x1c200 bytes before
// the cached end, reloads the cache: with mode 0 the routine returns at once with CF clear;
// otherwise a dirty cache is flushed by 0x0053859e (failure fails as above, success clears the
// dirty byte), the cache is invalidated, the range becomes esi .. esi + min(0x3b5380 - esi,
// [0x829c4c]) and 0x0053855e reads it into the cache (failure fails as above, success validates
// it); registers are restored around the reload. Then `rep movsb` copies the bytes from
// [0x829c44] + (esi - [0x829c54]) to edi and CF is cleared (other flags of the final add).
// FUNCTION: GP4 0x00538440
void __cdecl FUN_00538440(gp4::Regs* r) {
    const uint32_t count = r->ecx;
    const uint32_t offset = r->esi;
    uint32_t eax = r->eax;
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

    if (!fail) {
        bool reload = false;
        if (GP4_GLOBAL(uint8_t, 0x00829c50) != 0xffu) {
            reload = true;
        } else if (offset < GP4_GLOBAL(uint32_t, 0x00829c54)) {
            reload = true;
        } else {
            eax = offset + count;
            if (eax > GP4_GLOBAL(uint32_t, 0x00829c58)) {
                reload = true;
            } else if (r->edx == 0xffffffffu && GP4_GLOBAL(uint8_t, 0x00829c68) != 0u) {
                eax += 0x1c200u;
                if (eax >= GP4_GLOBAL(uint32_t, 0x00829c58)) {
                    reload = true;
                }
            }
        }
        if (reload) {
            const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00829c68);
            if (mode == 0u) {
                r->eax = eax;
                r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0u) & ~gp4::flags::CF);
                return;
            }
            if (GP4_GLOBAL(uint8_t, 0x00829c51) == 0xffu) {
                gp4::Regs c = *r;
                c.eax = eax;
                c.esi = GP4_GLOBAL(uint32_t, 0x00829c44);
                c.edi = GP4_GLOBAL(uint32_t, 0x00829c54);
                c.ecx = GP4_GLOBAL(uint32_t, 0x00829c58) - c.edi;
                c.eflags = gp4::flags::merge(r->eflags,
                    gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00829c58), c.edi));
                gp4::call_regs(0x0053859e, c);
                if ((c.eflags & gp4::flags::CF) != 0u) {
                    arith = c.eflags;
                    fail = true;
                } else {
                    GP4_GLOBAL(uint8_t, 0x00829c51) = 0u;
                }
            }
            if (!fail) {
                GP4_GLOBAL(uint8_t, 0x00829c50) = 0u;
                GP4_GLOBAL(uint32_t, 0x00829c54) = offset;
                uint32_t span = 0x3b5380u - offset;
                if (span > GP4_GLOBAL(uint32_t, 0x00829c4c)) {
                    span = GP4_GLOBAL(uint32_t, 0x00829c4c);
                }
                gp4::Regs c = *r;
                c.eax = offset + span;
                c.ecx = span;
                GP4_GLOBAL(uint32_t, 0x00829c58) = c.eax;
                c.edi = GP4_GLOBAL(uint32_t, 0x00829c44);
                c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, span));
                gp4::call_regs(0x0053855e, c);
                if ((c.eflags & gp4::flags::CF) != 0u) {
                    arith = c.eflags;
                    fail = true;
                } else {
                    GP4_GLOBAL(uint8_t, 0x00829c50) = 0xffu;
                }
            }
        }
    }

    if (fail) {
        gp4::Regs c = *r;
        c.eax = eax;
        c.esi = 0x00538540u;
        c.eflags = gp4::flags::merge(r->eflags, arith);
        gp4::call_regs(0x0054e232, c);
        r->eax = eax;
        r->eflags = gp4::flags::merge(r->eflags, c.eflags | gp4::flags::CF);
        return;
    }

    const uint32_t relative = offset - GP4_GLOBAL(uint32_t, 0x00829c54);
    const uint32_t source = relative + GP4_GLOBAL(uint32_t, 0x00829c44);
    arith = gp4::flags::add32(relative, GP4_GLOBAL(uint32_t, 0x00829c44));
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(source));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    for (uint32_t k = count; k != 0u; --k) {
        *dst++ = *src++;
    }
    r->eax = source;
    r->ecx = 0u;
    r->esi = source + count;
    r->edi += count;
    r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::flags::CF);
}
GP4_IMPL(0x00538440, FUN_00538440, "regs(eax:u32, ecx:u32[0..0x200], edx:u32{0xffffffff,0,1}, esi:u32[0..0x400], edi:ptr[0x200]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=cache_0053832b")
