#ifndef LIBPAS_DEBUG_H
#define LIBPAS_DEBUG_H

#define F __func__
#ifdef MODE_DEBUG
#define NEED_DEBUG 1
#else
#define NEED_DEBUG 0
#endif

#define printde(fmt, ...) do { if (NEED_DEBUG) fprintf(stderr, "%s:%s():%d: " fmt, __FILE__,  __func__,__LINE__, __VA_ARGS__); } while (0)
#define putsde(str) do { if (NEED_DEBUG) fprintf(stderr, "%s:%s():%d: %s",__FILE__,  __func__,__LINE__, str); } while (0)
#define perrord(str) do { if (NEED_DEBUG) {fprintf(stderr, "%s:%s():%d: ",__FILE__,  __func__,__LINE__);perror(str);} } while (0)
#define printdo(fmt, ...) do { if (NEED_DEBUG) fprintf(stdout, fmt, __VA_ARGS__); } while (0)
#define putsdo(str) do { if (NEED_DEBUG) fputs(str,stdout); } while (0)
#define perrorl(str) {fprintf(stderr, "%s:%s():%d: ",__FILE__,  __func__,__LINE__);perror(str);}
#define putsel(str) {fprintf(stderr, "%s:%s():%d: %s",__FILE__,  __func__,__LINE__, str);}
#endif 

