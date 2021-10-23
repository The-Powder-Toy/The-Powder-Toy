#include "tpt-thread-local.h"

#ifdef __MINGW32__
# include <pthread.h>
# include <cstdlib>
# include <cassert>

void *ThreadLocalCommon::Get() const
{
	// https://stackoverflow.com/questions/16552710/how-do-you-get-the-start-and-end-addresses-of-a-custom-elf-section
	extern ThreadLocalCommon __start_tpt_tls;
	extern ThreadLocalCommon __stop_tpt_tls;
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	static pthread_key_t key;

	struct ThreadLocalEntry
	{
		void *ptr;
	};

	auto *staticsBegin = &__start_tpt_tls;
	auto *staticsEnd = &__stop_tpt_tls;
	pthread_once(&once, []() -> void {
		assert(!pthread_key_create(&key, [](void *opaque) -> void {
			auto *staticsBegin = &__start_tpt_tls;
			auto *staticsEnd = &__stop_tpt_tls;
			auto staticsCount = staticsEnd - staticsBegin;
			auto *liveObjects = reinterpret_cast<ThreadLocalEntry *>(opaque);
			if (liveObjects)
			{
				for (auto i = 0; i < staticsCount; ++i)
				{
					if (liveObjects[i].ptr)
					{
						staticsBegin[i].dtor(liveObjects[i].ptr);
						free(liveObjects[i].ptr);
					}
				}
				free(liveObjects);
			}
		}));
	});
	auto *liveObjects = reinterpret_cast<ThreadLocalEntry *>(pthread_getspecific(key));
	if (!liveObjects)
	{
		auto staticsCount = staticsEnd - staticsBegin;
		liveObjects = reinterpret_cast<ThreadLocalEntry *>(calloc(staticsCount, sizeof(ThreadLocalEntry)));
		assert(liveObjects);
		assert(!pthread_setspecific(key, reinterpret_cast<void *>(liveObjects)));
	}
	auto idx = this - staticsBegin;
	auto &entry = liveObjects[idx];
	if (!entry.ptr)
	{
		entry.ptr = malloc(staticsBegin[idx].size);
		assert(entry.ptr);
		staticsBegin[idx].ctor(entry.ptr);
	}
	return entry.ptr;
}
#endif
