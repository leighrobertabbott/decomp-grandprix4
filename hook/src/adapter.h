#pragma once
#include <stdint.h>
#include <gp4/gp4.h>

// Returns the address of an executable thunk that, when jumped to in place of an
// original register-convention routine, packs the machine state into gp4::Regs,
// calls r.impl, and unpacks the outputs (GPRs, EFLAGS, x87 stack per r.spec).
uint32_t make_regs_thunk(const gp4::ImplRecord& r);
