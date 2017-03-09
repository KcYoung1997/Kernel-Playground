void memset(void* bufptr, unsigned char value, int size) {
	unsigned char* buf = (unsigned char *)bufptr;
	for (int i = 0; i < size; i++) {
		buf[i] = value;
	}
}
