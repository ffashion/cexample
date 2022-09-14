# lex 
1. lex function
    1. yylex()
        1. yylex_init()
        2. yylex_init_extra()
        3. yylex_destroy()
    2. yyerror()
    4. yywrap()
        1. 必须由用户提供。
        2. 可以使用 %option noyywrap 来始终返回1
    5. yyinput()
        1. 在c++里面会使用yyinput() , 在C里面是input()
    6. yyterminate()
2. lex var
    1. lex globle var 
        1. yylval
            1. 默认情况下yylval
        2. yyout
        3. yyextra
        4. yylloc 
        5. yytext
        6. yyleng
        7. yylineno
        8. yyin
# bison
1. bison function
    1. yyparse()
        1. 在bison中 可以通过%parse-param { std::unique_ptr<std::string> &ast } 设置yyparse的参数
        2. %define parser_class_name {seclang_parser}  设置生成的c文件的名字 以及class名字
1. locations 
2. options

# 一般的bison和lex的执行过程
1. 首先会调用bison的parser，然后parser 会调用flex的token分析。。。