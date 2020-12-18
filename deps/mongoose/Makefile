CFLAGS = -Os -Wall -I../mbedtls/include -DMG_ENABLE_DIRECTORY_LISTING \
	-DMG_ENABLE_MBEDTLS -DMG_ENABLE_FS -DMG_ENABLE_LOG=0

mongoose.o: mongoose.h mongoose.c

clean:
	$(RM) -f *.o
