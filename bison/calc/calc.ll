%{
    //这个文件是使用正则 进行输入匹配 然后返回结果的
    #include <stdlib.h>
    #include <stdio.h>
    #include "calc.hh" //由yacc -d 生成
%}


%option noyywrap
 //用%x <state name>的方式来定义新的独占起始状态
 //用%s <state name>可以定义一个非独占起始状态

NUMBER [0-9]+(\.[0-9]+)?([eE][0-9]+)?
OPERATOR  [-+()*/]
END  [ \t\f\v\n]




%%

%{
    //每次yylex 都会被调用 也就是每解析一个token 都会被调用
    static int i = 0;
    printf("number :%d\n",i++);
%}
  ////yylval.f 为union 的float变量类型
{NUMBER} { yylval.f = atof(yytext); return NUM;}
{OPERATOR} { return yytext[0];}
{END} {;}

%%