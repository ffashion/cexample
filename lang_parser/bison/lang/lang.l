%option noyywrap
%option nounput
%option noinput

%{
// this parser by flex 词法规则, 这个注释不能放在文件开头
#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "lang.tab.hpp"

// define YY_DECL to modify yylex return type, function name, and para...
#define YY_DECL int yylex (std::unique_ptr<std::string> &ast)
YY_DECL;

using namespace std;

%}

/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

%{
    // Code run each time a pattern is matched.
    // we can't call printf directly, must use  constructor, destructor, or type conversion...
    #define YY_USER_ACTION printf("%d len string, matched\n", yyleng);
%}

%%

%{
    // Code run each time yylex is called.
    static int __mycount = 0;
    __mycount++;
    printf("called yylex...%d\n",__mycount);
%}

{WhiteSpace}    { /* ignore, do not */ }
{LineComment}   { /* ignore, do not */ }

"int"           { return INT; }
"return"        { return RETURN; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

.               { return yytext[0]; }

%%
