#ifndef USER_MEMALLOC_H

#define USER_MEMALLOC_H

// This header file affords the developer to trap all memory allocations performed by the JobSwarm library
// and redirect them to their own routines, using placement new, etc.

#define MEMALLOC_NEW(x) new x
#define MEMALLOC_NEW_ARRAY(x,y) new x

#define MEMALLOC_DELETE(x,y) delete y;
#define MEMALLOC_DELETE_ARRAY(x,y) delete []y;

#define MEMALLOC_MALLOC_TYPE(x,t,f,l) ::malloc(x)
#define MEMALLOC_MALLOC_TAGGED(x,t) ::malloc(x)
#define MEMALLOC_MALLOC(x) ::malloc(x)
#define MEMALLOC_FREE(x) ::free(x)
#define MEMALLOC_REALLOC(x,y) ::realloc(x,y)
#define MEMALLOC_HEAP_CHECK()
#define MEMALLOC_REPORT(x,y,z) x; y; z;
#define MEMALLOC_GET_HEAP_SIZE(x) (x = 0)
#define MEMALLOC_SET_SEND_TEXT_MESSAGE(x)
#define MEMALLOC_MALLOC_ALIGN(x,y,t,f,l) mallocAlign(x,y)
#define MEMALLOC_FRAME_BEGIN(x) x;
#define MEMALLOC_FRAME_END(x,y) x; y;
#define MEMALLOC_SET_MEMORY_SHUTDOWN(x) x;
#define MEMALLOC_CORE_DUMP(x,y,z) x; y; z;
#define MEMALLOC_PROCESS_CORE_DUMP(x,y,z)
#define MEMALLOC_HEAP_COMPACT()

#endif
