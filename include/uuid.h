#ifndef INCLUDE_UUID_H_
#define INCLUDE_UUID_H_

#ifdef __sgi
#define RAND_LENGTH 2
#elif __NetBSD__
#define RAND_LENGTH 4
#elif __linux__
#define RAND_LENGTH 4
#else
#warning "Unknown platform, assuming 4 bytes of randomness from rand()"
#define RAND_LENGTH 4
#endif

typedef unsigned char uuid_t[16];

#define uuid_generate(out) uuid_generate_random(out)

extern void uuid_generate_random(uuid_t out);
extern void uuid_unparse(const uuid_t uuid, char *out);
extern void uuid_copy(uuid_t dst, const uuid_t src);
extern void uuid_parse(const char *in, uuid_t uuid);

#endif // INCLUDE_UUID_H_
