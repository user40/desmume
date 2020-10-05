#include "stacktrace.h"

// Global variable
CallStack callstack(&NDS_ARM9);
//CallStack callstackARM7(&NDS_ARM7);

// Macro
#define SP this->cpu->R[13]
#define LR this->cpu->R[14]
#define PC this->cpu->R[15]
#define THUMB_FLAG this->cpu->CPSR.bits.T
#define FRAME_ADR(i) this->frame[i].frame_adr


CallStack::CallStack(const armcpu_t* cpu) : cpu(cpu), top(0), frame() {}

void CallStack::reset()
{
	this->top = 0;
	for (int i = 0; i < stacksize; i++)
	{
		this->frame[i] = stack_frame{};
	}
}

void CallStack::on_lr_pushed()
{
	//if (this->cpu->CPSR.bits.mode == Mode::IRQ)
	//{
	//	return;
	//}
	const auto sp = SP;

	auto i = this->top;
	// The first time, this clauses is skipped because F_ADR(i) == 0 && i == 0.
	if (sp < FRAME_ADR(i))
	{
		i++;
		if (i >= stacksize)
		{
			return;
		}
	}
	else
	{
		while (sp > FRAME_ADR(i) && i > 0)
		{
			--i;
		}
	}

	this->top = i;
	this->frame[i] = stack_frame{ sp, LR, PC, THUMB_FLAG };
}

int CallStack::pass_to_lua(lua_State* L)
{
	const auto cs = this->get_callstack();

	lua_newtable(L);  // create table
	int i = 1;
	for (const auto& e : cs)
	{
		lua_pushnumber(L, i);  // index

		lua_newtable(L);  // stack frame
		lua_pushnumber(L, e.frame_adr);
		lua_setfield(L, -2, "frame_adr");
		lua_pushnumber(L, e.caller);
		lua_setfield(L, -2, "caller");
		lua_pushnumber(L, e.callee);
		lua_setfield(L, -2, "callee");
		lua_pushboolean(L, e.is_THUMB);
		lua_setfield(L, -2, "is_THUMB");

		lua_settable(L, -3);  // table[i] = stack frame
		i++;
	}
	return 1;  // The number of return values
}

std::vector<stack_frame> CallStack::get_callstack()
{
	std::vector<stack_frame> cs;
	for (int i = this->top; i >= 0; i--)
	{
		if (SP > FRAME_ADR(i))
		{
			continue;
		}

		auto f = frame[i];
		f.caller -= (f.is_THUMB ? 4 : 8);
		cs.push_back(f);
	}

	return cs;
}