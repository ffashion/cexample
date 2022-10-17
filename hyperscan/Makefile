all:
	# error 
	# g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan.c -o hyperscan -lchimera -lpcre -lhs

	g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan.c -o hyperscan /home/parallels/project/hyperscan/install/lib/libchimera.a /home/parallels/project/hyperscan/install/lib/libhs.a /home/parallels/project/hyperscan/install/lib/libpcre.a
	
	# default use static lib(-Wl,-static), and if we dont found static lib use dynamic lib(-Wl,-Bdynamic)
	g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan.c -o hyperscan -Wl,-static -lchimera -lpcre -lhs -Wl,-Bdynamic
	
	g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan_example.c -o hyperscan_example -lhs -lchimera
	g++ -I${HOME}/project/hyperscan/install/include -L${HOME}/project/hyperscan/install/lib/ -lasan -fsanitize=address -fno-omit-frame-pointer  -g hyperscan_lib.c -fPIC -shared -o hyperscan_lib.so -lchimera -lhs  -lpcre
