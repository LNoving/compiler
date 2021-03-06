%{
#include<stdio.h>

%}
quote \\\"
space " "
%%
\/\/.* ECHO; 
\/\*.*\*\/ ECHO;
\/\*(\n*.*\n*)*\*\/ ECHO;
\"[\t!#%&'()*+,-./0-9:; <=>?A-Z\[\]\^_a-z{|}]*\" ECHO;
\"[(\\n)(\\\n) \t!#%&'\(\)*+,\-./0-9:;<=>?A-Z\[\]\^_a-z{|}]*[\n \t!#%%&'\(\)*+,\-./0-9:;<=>?A-Z\[\]\^_a-z{|}]\" ECHO;
\"((\\\")|[(\\.)(\\\n) \t!#%&'\(\)*+,\-./0-9:;<=>?A-Z\[\]\^_a-z{|}])*[\n \t!#%%&'\(\)*+,\-./0-9:;<=>?A-Z\[\]\^_a-z{|}]\" ECHO;
#.* ECHO ;
[0-9a-zA-Z]auto    ECHO;
auto[0-9a-zA-Z]    ECHO;
auto    printf("AUTO");
[0-9a-zA-Z]break    ECHO;
break[0-9a-zA-Z]    ECHO;
break    printf("BREAK");
[0-9a-zA-Z]case    ECHO;
case[0-9a-zA-Z]    ECHO;
case    printf("CASE");
[0-9a-zA-Z]char    ECHO;
char[0-9a-zA-Z]    ECHO;
char    printf("CHAR");
[0-9a-zA-Z]const    ECHO;
const[0-9a-zA-Z]    ECHO;
const    printf("CONST");
[0-9a-zA-Z]continue    ECHO;
continue[0-9a-zA-Z]    ECHO;
continue    printf("CONTINUE");
[0-9a-zA-Z]default    ECHO;
default[0-9a-zA-Z]    ECHO;
default    printf("DEFAULT");
[0-9a-zA-Z]do    ECHO;
do[0-9a-zA-Z]    ECHO;
do    printf("DO");
[0-9a-zA-Z]double    ECHO;
double[0-9a-zA-Z]    ECHO;
double    printf("DOUBLE");
[0-9a-zA-Z]else    ECHO;
else[0-9a-zA-Z]    ECHO;
else    printf("ELSE");
[0-9a-zA-Z]enum    ECHO;
enum[0-9a-zA-Z]    ECHO;
enum    printf("ENUM");
[0-9a-zA-Z]extern    ECHO;
extern[0-9a-zA-Z]    ECHO;
extern    printf("EXTERN");
[0-9a-zA-Z]float    ECHO;
float[0-9a-zA-Z]    ECHO;
float    printf("FLOAT");
[0-9a-zA-Z]for    ECHO;
for[0-9a-zA-Z]    ECHO;
for    printf("FOR");
[0-9a-zA-Z]goto    ECHO;
goto[0-9a-zA-Z]    ECHO;
goto    printf("GOTO");
[0-9a-zA-Z]if    ECHO;
if[0-9a-zA-Z]    ECHO;
if    printf("IF");
[0-9a-zA-Z]int    ECHO;
int[0-9a-zA-Z]    ECHO;
int    printf("INT");
[0-9a-zA-Z]long    ECHO;
long[0-9a-zA-Z]    ECHO;
long    printf("LONG");
[0-9a-zA-Z]register    ECHO;
register[0-9a-zA-Z]    ECHO;
register    printf("REGISTER");
[0-9a-zA-Z]return    ECHO;
return[0-9a-zA-Z]    ECHO;
return    printf("RETURN");
[0-9a-zA-Z]short    ECHO;
short[0-9a-zA-Z]    ECHO;
short    printf("SHORT");
[0-9a-zA-Z]signed    ECHO;
signed[0-9a-zA-Z]    ECHO;
signed    printf("SIGNED");
[0-9a-zA-Z]sizeof    ECHO;
sizeof[0-9a-zA-Z]    ECHO;
sizeof    printf("SIZEOF");
[0-9a-zA-Z]static    ECHO;
static[0-9a-zA-Z]    ECHO;
static    printf("STATIC");
[0-9a-zA-Z]struct    ECHO;
struct[0-9a-zA-Z]    ECHO;
struct    printf("STRUCT");
[0-9a-zA-Z]switch    ECHO;
switch[0-9a-zA-Z]    ECHO;
switch    printf("SWITCH");
[0-9a-zA-Z]typedef    ECHO;
typedef[0-9a-zA-Z]    ECHO;
typedef    printf("TYPEDEF");
[0-9a-zA-Z]union    ECHO;
union[0-9a-zA-Z]    ECHO;
union    printf("UNION");
[0-9a-zA-Z]unsigned    ECHO;
unsigned[0-9a-zA-Z]    ECHO;
unsigned    printf("UNSIGNED");
[0-9a-zA-Z]void    ECHO;
void[0-9a-zA-Z]    ECHO;
void    printf("VOID");
[0-9a-zA-Z]volatile    ECHO;
volatile[0-9a-zA-Z]    ECHO;
volatile    printf("VOLATILE");
[0-9a-zA-Z]while    ECHO;
while[0-9a-zA-Z]    ECHO;
while    printf("WHILE");

