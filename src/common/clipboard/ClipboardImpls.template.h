#ifdef CLIPBOARD_IMPLS_DECLARE
# define IMPL_DEFINE(subsystem, factory) std::unique_ptr<ClipboardImpl> factory();
#endif
#ifdef CLIPBOARD_IMPLS_DEFINE
# define IMPL_DEFINE(subsystem, factory) { subsystem, factory },
#endif

@impl_defs@

#undef IMPL_DEFINE
