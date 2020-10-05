#include "types.h"
#include <vector>
#include <armcpu.h>

extern "C" {
#include "lua.h"
};

static const size_t stacksize = 0x200;

struct stack_frame {
	u32 frame_adr;
	u32 caller;
	u32 callee;
	u32 is_THUMB;
};

class CallStack {
	const armcpu_t* cpu;
	int top;
	stack_frame frame[stacksize];

public:
	CallStack(const armcpu_t* cpu);
	void reset();
	void on_lr_pushed();
	int pass_to_lua(lua_State* L);
	std::vector<stack_frame> get_callstack();
};

// Global variable
extern CallStack callstack;