all:
	gcc -g dlopen.c -o dlopen -ldl
	
	#不能使用-c 
	gcc -g -fPIC -shared lib.c -o lib.so