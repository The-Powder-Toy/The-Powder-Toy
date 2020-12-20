#ifdef ELEMENT_NUMBERS_CALL
# define ELEMENT_DEFINE(name, id) elements[id].Element_ ## name ()
#endif
#ifdef ELEMENT_NUMBERS_DECLARE
# define ELEMENT_DEFINE(name, id) void Element_ ## name ()
#endif
#ifdef ELEMENT_NUMBERS_ENUMERATE
# define ELEMENT_DEFINE(name, id) constexpr int PT_ ## name = id
#endif

@element_defs@

#undef ELEMENT_DEFINE
