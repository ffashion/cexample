
head := dict.h mt19937-64.h 

obj := dict.o mt19937-64.o main.o siphash.o
	
cflags := -Wall -Wextra -Werror -Wno-implicit-fallthrough -Wno-stack-protector
cflags_debug := -Wall -Wextra -Werror -Wno-implicit-fallthrough -Wno-stack-protector  -fsanitize=address -fno-omit-frame-pointer -fsanitize=leak 

all: main

%.o:%.c $(head)
	@ echo -e "  \e[32mCC\e[0m	" $@
	@ gcc -std=gnu11 -I ./ -o $@ -c $< -g3 $(cflags)



main: $(obj)
	@ echo -e "  \e[34mMKELF\e[0m	" $@
	@ gcc  -o $@ $^ -g3 $(cflags_debug)


clean:
	@ rm -f $(obj) main
