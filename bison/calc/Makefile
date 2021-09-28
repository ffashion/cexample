# all:
# 	yacc calc.yy -d 
# 	lex  calc.ll 
# 	gcc y.tab.c lex.yy.c -o calc
all:
	yacc calc.yy -o calc.cc -d 
	lex -o calc_lex.cc calc.ll
	g++ calc.cc calc_lex.cc -o calc
clean:
	rm -rf calc.cc calc.hh calc_lex.cc  y.tab.c lex.yy.c  y.tab.h calc