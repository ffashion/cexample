openssldir=/home/fashion/project/openssl/install/

CFLAGS = -g -fPIC 
CFLAGS += -I$(openssldir)/include -L$(openssldir)/lib -Wl,-rpath=$(openssldir)/lib
all:
	gcc $(CFLAGS) rsa_sign.c -o rsa_sign $(openssldir)/lib/libcrypto.a -ldl -lpthread
	gcc $(CFLAGS) ecc_sign.c -o ecc_sign $(openssldir)/lib/libcrypto.a -ldl -lpthread
	gcc  -Wall  $(CFLAGS) -g sm2_sign.c -o sm2_sign $(openssldir)/lib/libcrypto.a -ldl -lpthread
	gcc -Wall $(CFLAGS) rsa_verify.c -o rsa_verify $(openssldir)/lib/libcrypto.a -ldl -lpthread
clean: 
	rm -rf rsa_sign ecc_sign rsa_verify sm2_sign