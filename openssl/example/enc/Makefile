openssldir=/home/fashion/project/openssl/install/

CFLAGS = -O -g -fPIC 
CFLAGS += -I$(openssldir)/include -L$(openssldir)/lib -Wl,-rpath=$(openssldir)/lib
all:
	gcc $(CFLAGS) rsa_enc.c -o rsa_enc $(openssldir)/lib/libcrypto.a -ldl -lpthread
	gcc $(CFLAGS) ec_enc.c -o ec_enc $(openssldir)/lib/libcrypto.a -ldl -lpthread
	gcc  -Wall  $(CFLAGS) -g sm2_enc.c -o sm2_enc $(openssldir)/lib/libcrypto.a -ldl -lpthread
clean: 
	rm -rf rsa_enc ec_enc sm2_enc