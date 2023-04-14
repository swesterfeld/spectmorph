// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifdef SM_MALLOC_TRACER
#include <dlfcn.h>

void *(*old_malloc)(size_t size) = nullptr;
void *(*old_realloc)(void *ptr, size_t size) = nullptr;
void *(*old_memalign)(size_t alignment, size_t size) = nullptr;
void *(*old_free)(void *ptr) = nullptr;

int malloc_count = 0;
int realloc_count = 0;
int memalign_count = 0;
int free_count = 0;
int malloc_counting = 0;

extern "C" {
void*
malloc (size_t size)
{
  if (!old_malloc)
    old_malloc = reinterpret_cast <void *(*)(size_t)> (dlsym(RTLD_NEXT, "malloc"));

  if (malloc_counting && SpectMorph::sm_dsp_thread())
    malloc_count++;

  return old_malloc(size);
}

void *
realloc (void *ptr, size_t size)
{
  if (!old_realloc)
    old_realloc = reinterpret_cast <void *(*)(void *, size_t)> (dlsym(RTLD_NEXT, "realloc"));

  if (malloc_counting && SpectMorph::sm_dsp_thread())
    realloc_count++;

  return old_realloc (ptr, size);
}

void*
memalign (size_t alignment, size_t size)
{
  if (!old_memalign)
    old_memalign = reinterpret_cast <void *(*)(size_t, size_t)> (dlsym(RTLD_NEXT, "memalign"));

  if (malloc_counting && SpectMorph::sm_dsp_thread())
    memalign_count++;

  return old_memalign (alignment, size);
}

void
free (void *ptr)
{
  if (!old_free)
    old_free = reinterpret_cast <void *(*)(void *)> (dlsym(RTLD_NEXT, "free"));

  if (malloc_counting && SpectMorph::sm_dsp_thread())
    free_count++;

  old_free (ptr);
}

}

#endif

class MallocTracer
{
public:
  MallocTracer()
  {
#ifdef SM_MALLOC_TRACER
    malloc_counting = 1;
#endif
  }
  ~MallocTracer()
  {
#ifdef SM_MALLOC_TRACER
    malloc_counting = 0;
#endif
  }
  void
  print_stats()
  {
#ifdef SM_MALLOC_TRACER
    printf ("malloc: %d realloc: %d mem_align: %d free: %d\r", malloc_count, realloc_count, memalign_count, free_count);
    fflush (stdout);
#endif
  }
};

