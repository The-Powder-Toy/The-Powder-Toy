#ifdef TOOL_NUMBERS_CALL
# define TOOL_DEFINE(name, id) tools.push_back(SimTool()), tools.back().Tool_ ## name ()
#endif
#ifdef TOOL_NUMBERS_DECLARE
# define TOOL_DEFINE(name, id) void Tool_ ## name ()
#endif
#ifdef TOOL_NUMBERS_ENUMERATE
# define TOOL_DEFINE(name, id) constexpr int TOOL_ ## name = id
#endif

@tool_defs@

#undef TOOL_DEFINE
