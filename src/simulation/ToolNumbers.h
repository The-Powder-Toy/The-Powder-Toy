#ifdef TOOL_NUMBERS_CALL
# define TOOL_DEFINE(name, id) tools.push_back(SimTool()), tools.back().Tool_ ## name ()
#endif
#ifdef TOOL_NUMBERS_DECLARE
# define TOOL_DEFINE(name, id) void Tool_ ## name ()
#endif
#ifdef TOOL_NUMBERS_ENUMERATE
# define TOOL_DEFINE(name, id) constexpr int TOOL_ ## name = id
#endif

TOOL_DEFINE(HEAT, 0);
TOOL_DEFINE(COOL, 1);
TOOL_DEFINE(AIR, 2);
TOOL_DEFINE(VAC, 3);
TOOL_DEFINE(PGRV, 4);
TOOL_DEFINE(NGRV, 5);
TOOL_DEFINE(MIX, 6);
TOOL_DEFINE(CYCL, 7);

#undef TOOL_DEFINE
