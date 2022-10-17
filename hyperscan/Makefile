all:
	g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lpcre  -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan.c -o hyperscan -lhs -lchimera -lpcre
	g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lpcre -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan_example.c -o hyperscan_example -lhs -lchimera
	g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lpcre  -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan_lib.c -fPIC -shared -o hyperscan_lib.so -lchimera -lhs  -lpcre
