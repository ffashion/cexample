%{
    #include <stdio.h>
    #include <stdlib.h>
    extern int yylex();
    void yyerror(char *msg);
%}

%union {
    float f;
}

//yac 是一个自底向上解析器
//NUM是一个terminal

%token <f> NUM
%type <f> E T F


%%
S : E       {printf("%f \n",$1);}
  ;
E : E '+' T  {$$ = $1 + $3;}
  | E '-' T  {$$ = $1 - $3;}
  | T        {$$ = $1;}
  ;       
T : T '*' F {$$ = $1 * $3;}
  | T '/' F {$$ = $1 / $3;}
  | F       {$$ = $1;}
  ;
F : '(' E ')' {$$ = $2;}
  | '-' F     {$$ = -$2;}
  | NUM       {$$ = $1;}
  ;
%%

void yyerror(char *msg) {
    fprintf(stderr,"%s\n",msg);
    exit(-1);
}

int main(int argc,char *args[]){
    //yyac 生成的
    yyparse();
    return 0;
}