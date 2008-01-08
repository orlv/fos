#ifndef ENDIAN_H
#define ENDIAN_H
#define htons(a) ((((a) & 0xFF00) >> 8) | (((a) & 0xFF) << 8))
#define ntohs(a) htons(a)
/*
static inline u16_t htons(u16_t hostshort) {
	u16_t res;
	res = (hostshort & 0xFF00) >> 8;
	res |= (hostshort & 0xFF) << 8;
	return res;
}
*/
#endif
