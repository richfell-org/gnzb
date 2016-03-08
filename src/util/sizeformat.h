#ifndef __SIZE_FORMAT_HEADER__
#define __SIZE_FORMAT_HEADER__

unsigned long formatULongBytes(char *out, unsigned long number, bool addSuffix = true);
unsigned long formatUIntBytes(char *out, unsigned int number, bool addSuffix = true);
unsigned long formatLongBytes(char *out, long number, bool addSuffix = true);
unsigned long formatIntBytes(char *out, int number, bool addSuffix = true);

unsigned long formatBytePerSecond(char *out, unsigned long bytes, unsigned long seconds);
unsigned long formatBytePerSecond(char *out, double bytesPerSecond);

void formatTimeFromSeconds(char *out, unsigned long seconds);

#endif  /* __SIZE_FORMAT_HEADER__ */