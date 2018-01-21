/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */
%token IS CLASS VAR EXTENDS DEF OVERRIDE RETURN AS IF THEN ELSE AFF NEWC VOIDC INTC STRINGC ADD SUB MUL DIV CONCAT
%token OBJECT
%token<S> Id Classname 
%token<I> Cste
%token<C> RelOp


%nonassoc RelOp
%left ADD SUB 
%left MUL DIV
%left CONCAT
%nonassoc UNARY
%nonassoc '.'


%type <pT> 	TypeC ExtendsOpt AffOpt MethodeC
			TypeCOpt Expr LExprOpt LExpr
			ExprOperateur Instr LInstrOpt 
			LInstr Bloc 
			Envoi Selection
%type <pC> 	Class classTete classLOpt
%type <pM> 	DeclMethode
%type <pV> 	LDeclChamp DeclChamp Param LParamOpt
			LParam ValVar
%type <I> 	OverrideOpt

%{
#include "tp.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(const char *); /*const necessaire pour eviter les warning de string literals*/
%}

%%
Prog : classLOpt Bloc
;

classTete: CLASS Classname 						{/* $$ = makeClass($2); */}
;

classLOpt: Class classLOpt 						{/* $1->nextClass = $2; $$ = $1 ;*/}
| 												{/*$$ = NIL(Class)*/}
;

Class: Objet {} 
| classTete '(' LParamOpt ')' ExtendsOpt BlocOpt IS BlocObj 
												{/* initClasse($1, $3, $5, $6, $9, $10); $$ = $1; */}
;

Objet: OBJECT Classname IS BlocObj 				{	}
;
 LParamOpt: LParam 								{/*	$$ = $1; */}
|				  								{/* $$ = NIL(VarDecl); */}
;

LParam: Param ',' LParam 						{/* $1->nextParam = $3; $$=$1; */}
| Param            	     						{/* $$ = $1; */}
;

Param: VAR Id ':' TypeC ValVar 					{/* $$ = makeParam($1,$3,NIL(Tree),0); */}
| Id ':' TypeC ValVar 							{/* $$ = makeParam($1,$3,NIL(Tree),0); */}
;

ValVar: AFF Expr 								{	}
| 												{	}
;

LDeclChamp: DeclChamp LDeclChamp 				{/* $1->nextParam = $3; $$ = $1;*/}
| DeclChamp 									{/* $$ = $1; */}
;

DeclChamp: VAR Id ':' TypeC AffOpt ';' 			{/* $$ = makeParam($2, $4, $5, 2); */}
;

TypeC: INTC 									{/* $$ = makeLeafStr(APPC, "Integer"); */}    	//INTC STRINGC ET VOIDC peuvent êtres traités dans Classname en les mettant dans le même environnement
																								//(qui sera rentré à la main à la compilation)
| STRINGC 										{/* $$ = makeLeafStr(APPC, "String"); */}
| VOIDC 										{/*TODO; */}
| Classname 									{/* $$ = makeLeafStr(APPC, $1); */}
;

AffOpt: AFF Expr 								{/* $$ = makeTree(AEO, 2, NIL(Tree), $2); */}
|				 								{/* $$ = NIL(Tree); */}
;

/* method inutile
DeclMethodeLOpt: DeclMethode ';' DeclMethodeLOpt { $1->nextMethode = $2; $$ = $1; }
|  												{ $$ = NIL(Methode); }
;
*/

DeclMethode : OverrideOpt DEF Id '(' LParamOpt ')' ':' TypeC AFF Expr {/* $$ = makeMethode($1, $3, $5, $8, $10, 1); */}
| OverrideOpt DEF Id '(' LParamOpt ')' TypeCOpt IS Bloc {}

/*TODO Bloc { $$ = makeMethode($1, $3, $5, $7, $9, 1); }*/

;

TypeCOpt: ':' TypeC 							{/* $$ = makeTree(EAPPC, 2, NIL(Tree), $2); */}
| 												{/* $$ = NIL(Tree); */}
;

OverrideOpt: OVERRIDE 							{/* $$ = TRUE; */}
|     				  							{/* $$ = FALSE; */}
;

ExtendsOpt: EXTENDS Classname '(' LExprOpt ')'  {/* TreeP nC = makeLeafStr(APPC, $2); $$ = makeTree(EXTO, 2, nC, $4); */}
|					{/* $$ = NIL(Tree); */}
;

Expr: 		Cste 								{/* $$ = makeLeafInt(ECONST, $1); */}
| '(' Expr ')'  								{/* $$ = makeTree(EPEXP, 2, NIL(Tree), $2); */}
| '(' TypeC Id ')' 								{/* $$ = makeTree(ECAST, 2, NIL(Tree), $1); */}	//CCast
| Selection    									{/* $$ = makeTree(ESEL, 2, NIL(Tree), $1); */}
| NEWC Classname '(' LExprOpt ')' 				{/* $$ = makeTree(EINSTA, 2, NIL(Tree), $1); */}   //Instanciation
| Envoi  										{/* $$ = makeTree(EENV, 2, NIL(Tree), $1); */}
| ExprOperateur 								{/* $$ = makeTree(EOPER, 2, NIL(Tree), $1); */}
| TypeC											{	}
;



LExprOpt: LExpr 								{/* $$ = makeTree(EEXPO, 2, NIL(Tree), $1); */}
| 												{/* $$ = NIL(Tree); */}
;

LExpr: Expr ',' LExpr 							{/* $$ = makeTree(LEXP, 2, $1, $3); */}
| Expr 			  								{/* $$ = makeTree(EEXP, 2, NIL(Tree), $1); */}
;

ExprOperateur: Expr ADD Expr 					{/* $$ = makeTree(EADD, 2, $1, $3); */}
| Expr SUB Expr 								{/* $$ = makeTree(EMINUS, 2, $1, $3); */}
| Expr MUL Expr 								{/* $$ = makeTree(EMUL, 2, $1, $3); */}
| Expr DIV Expr  								{/* $$ = makeTree(EQUO, 2, $1, $3); */}
| SUB Expr %prec UNARY 							{/* $$ = makeTree(EMINUS, 2, makeLeafInt(CONST, 0), $2); */}
| ADD Expr %prec UNARY 							{/* $$ = makeTree(EADD, 2, makeLeafInt(CONST, 0), $2); */}      
| Expr CONCAT Expr 								{/* $$ = makeTree(EAND, 2, $1, $3); */}
| Expr RelOp Expr 								{/* $$ = makeTree($2, 2, $1, $3); */}
;

Instr : Expr ';' 								{/* $$ = makeTree(EEXP, 2, NIL(Tree), $1); */}
| Bloc  										{/*$$ = makeTree(EBLOC, 2, NIL(Tree), $1); */}
| RETURN ';' 									{/* $$ = makeLeafStr(ERET, NIL(char)); */}
| Selection AFF Expr ';' 						{/* $$ = makeTree(EAFF, 2, $1, $3); */}
| IF Expr THEN Instr ELSE Instr 				{/* $$ = makeTree(ITE, 3, $2, $4, $6); */}
;

LInstrOpt: LInstr 								{/*$$=makeTree(ILINSTO, 2, $1, $2);*/}
|  			 									{/* $$ = NIL(Tree); */}
;

LInstr : Instr LInstr 							{/*$$ = makeTree(LINST, 2, $1, $2);*/}
| Instr		 									{/*$$ = makeTree(EINST, 2, $1, NIL(Tree)); */}
;

BlocOpt: Bloc {}
| {}
;

Bloc : '{' Contenu '}'    						{/* $$ = makeTree(EIB, 2, NIL(Tree), $2); */}
;

BlocObj: '{' LDeclChampMethodOpt '}' 			{	}
;

LDeclChampMethodOpt: LDeclChampMethod 			{	}
| 												{	}
;

LDeclChampMethod: LDeclChampMethod DeclChampMethod {	}
| DeclChampMethod 								{	}
;

DeclChampMethod: DeclChamp 						{	}
| DeclMethode 									{	}
;

Contenu : LInstrOpt  							{/* $$ = makeTree(LINSTO, 2, NIL(Tree), $1); */}
| LDeclChamp IS LInstr 							{/* $$ = makeTree(EIS, 2, $1, $3); */}
;

Envoi: Expr '.' MethodeC 						{/* $$ = makeTree(EEXPA, 2, $1, $3); */}
;

MethodeC: Id '(' LExprOpt ')' 					{	}
;

Selection: Expr '.' Id 							{/* $$ = makeTree(EEXPI, 2, $1, makeLeafStr(IDVAR, $3)); */}
| Id 				   							{/* $$ = makeLeafStr(IDVAR, $1); */}
;
