
#ifndef  __LOG_H_
#define  __LOG_H_

#define WARNING(fmt, msg ...) \
    do { \
        fprintf(stderr, "WARNING (%s +%d %s) " fmt, __FILE__, __LINE__, __func__, ##msg); \
    } while (0)

#define FATAL(fmt, msg ...) \
    do { \
        fprintf(stderr, "FATAL (%s +%d %s) " fmt, __FILE__, __LINE__, __func__, ##msg); \
    } while (0)

#define NOTICE(fmt, msg ...) \
    do { \
        fprintf(stderr, "NOTICE (%s +%d %s) " fmt,__FILE__, __LINE__, __func__, ##msg); \
    } while (0)

#define TRACE(fmt, msg ...) \
    do { \
        fprintf(stderr, "TRACE (%s +%d %s) " fmt,__FILE__, __LINE__, __func__, ##msg); \
    } while (0)

#define DEBUG(fmt, msg ...) \
    do { \
        fprintf(stderr, "DEBUG (%s +%d %s) " fmt,__FILE__, __LINE__, __func__, ##msg); \
    } while (0)














#endif  //__LOG_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
