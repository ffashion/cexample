all:
	flex -o lang.lex.cpp lang.l
	bison -d -o lang.tab.cpp lang.y --debug  # 此时 bison 还会生成 `lang.tab.hpp`
	g++ -std=c++11 -g lang.cpp lang.tab.cpp lang.lex.cpp -o lang
build:
	g++ -std=c++11 lang.cpp lang.tab.cpp lang.lex.cpp -o lang
clean:
	rm -rf lang lang.tab.cpp lang.lex.cpp lang.tab.hpp lang.dSYM