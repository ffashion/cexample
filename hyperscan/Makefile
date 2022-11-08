all:
	g++ -I/home/fashion/project/hyperscan/install/include -L/home/fashion/project/hyperscan/install/lib64 -lpcre  -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan.c -o hyperscan -lhs -lchimera -lpcre
	gcc -I/home/fashion/project/hyperscan/install/include -L/home/fashion/project/hyperscan/install/lib64/ -lpcre -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan_example.c -o hyperscan_example -lhs -lchimera
	gcc -I/home/fashion/project/hyperscan/install/include -L/home/fashion/project/hyperscan/install/lib64/ -lpcre  -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan_lib.c -fPIC -shared -o hyperscan_lib.so -lchimera -lhs  -lpcre
