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

%type <pT> 	TypeC AffOpt MethodeC
			TypeCOpt Expr LExprOpt LExpr
			ExprOperateur Instr LInstrOpt 
			LInstr Bloc BlocOpt Contenu
			Envoi Selection Prog
			LDeclChamp DeclChamp ExtendsOpt
			ValVar LDeclChampMethodOpt

/*%type <pB> 	Bloc BlocOpt*/

%type <pBO>	BlocObj

%type <pOI> ObjetIsole 					/*objet isole*/

%type <pC> 	Class LClassOpt

%type <pM> 	DeclMethode

%type <pV> 	Param LParamOpt LParam 		/*type LParamP*/


%type <I> 	OverrideOpt

%{
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(const char *); /*const necessaire pour eviter les warning de string literals*/
%}

%%
Prog : LClassOpt Bloc 						{$$ = makeTree(YPROG,2,$1,$2); printTree($$);}
;

Class: ObjetIsole									{ } 

| CLASS Classname '(' LParamOpt ')' ExtendsOpt BlocOpt IS BlocObj
													{ }
;



ObjetIsole : OBJECT Classname IS BlocObj			{ }
;



LClassOpt: Class LClassOpt 							{/*$1->nextClasse = $2; 	$$ = $1 ;	printf("NextClasse. \n"); */}

| 												{$$ = NIL(Tree); }
;


 LParamOpt: LParam 									{ /*$$ = $1; 						printf("LParam \n");	*/		}
|				  								{$$ = NIL(Tree); }
;

LParam: Param ',' LParam 							{/*$1->nextAttribut = $3; $$=$1; 	printf("Param , LParam \n"); */	}
| Param            	     							{ /*$$ = $1; 	printf("Param \n");*/				}
;

Param: VAR Id ':' TypeC ValVar 						{		}
| Id ':' TypeC ValVar 								{/* $$ = makeParam($1,$3,NIL(Tree),0); */}
;

ValVar: AFF Expr 									{/*$$ = $2;*/	}
| 												{$$ = NIL(Tree); }
;

LDeclChamp: DeclChamp LDeclChamp 				{$$ = makeTree(LDECLC,2,$1,$2);}
| DeclChamp 									{$$ = $1; }
;

DeclChamp: VAR Id ':' TypeC AffOpt ';' 			{$$ = makeTree(YDECLC,3, $2, $4, $5);}
;

TypeC: INTC 									{$$ = makeLeafStr(TINTC, "Integer");}    	//INTC STRINGC ET VOIDC peuvent êtres traités dans Classname en les mettant dans le même environnement
																								//(qui sera rentré à la main à la compilation)
| STRINGC 										{$$ = makeLeafStr(TSTRINGC, "String"); }
| VOIDC 										{$$ = NIL(Tree);}
| Classname 									{$$ = makeLeafStr(TSTRINGC, $1); }
;
		
AffOpt: AFF Expr 									{/* $$ = makeTree(AEO, 2, NIL(Tree), $2); */}
|				 								{ $$ = NIL(Tree); }
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

TypeCOpt: ':' TypeC 								{/* $$ = makeTree(EAPPC, 2, NIL(Tree), $2); */}
| 												{ $$ = NIL(Tree); }
;
	
OverrideOpt: OVERRIDE 								{/* $$ = TRUE; */}
|     				  								{/* $$ = FALSE; */}
;

ExtendsOpt: EXTENDS Classname '(' LExprOpt ')'  {  $$ = makeTree(YEXT, 2, makeLeafStr(TSTRINGC, $2), $4); }
|												{ $$ = NIL(Tree); }
;

Expr: Cste 										{$$ = makeLeafInt(ECONST, $1); }
| '(' Expr ')'  								{$$ = $2; }
| '(' TypeC Id ')' 								{$$ = makeTree(ECAST, 2, $2, $3); }	
| Selection    									{$$ = $1; }
| NEWC Classname '(' LExprOpt ')' 				{$$ = makeTree(EINST, 2, $2, $4); }   
| Envoi  										{$$ = $1;}
| ExprOperateur 								{$$ = $1;}
| TypeC											{$$ = $1;}
;

LExprOpt: LExpr 								{ $$ = $1; /*$$ = makeTree(YLEXPR, 2, NIL(Tree), $1); */}
| 												{ $$ = NIL(Tree); }
;

LExpr: Expr ',' LExpr 								{/* $$ = makeTree(YLEXPR, 2, $1, $3);*/ }							/*TODO BIzarre*/
| Expr 			  									{ $$ = $1; /*$$ = makeTree(EEXP, 2, NIL(Tree), $1);*/ }
;

ExprOperateur: Expr ADD Expr 						{/* $$ = makeTree(EADD, 2, $1, $3); */}
| Expr SUB Expr 									{/* $$ = makeTree(EMINUS, 2, $1, $3); */}
| Expr MUL Expr 									{/* $$ = makeTree(EMUL, 2, $1, $3); */}
| Expr DIV Expr  									{/* $$ = makeTree(EQUO, 2, $1, $3); */}
| SUB Expr %prec UNARY 								{/* $$ = makeTree(EMINUS, 2, makeLeafInt(CONST, 0), $2); */}
| ADD Expr %prec UNARY 								{/* $$ = makeTree(EADD, 2, makeLeafInt(CONST, 0), $2); */}      
| Expr CONCAT Expr 									{/* $$ = makeTree(EAND, 2, $1, $3); */}
| Expr RelOp Expr 									{/* $$ = makeTree($2, 2, $1, $3); */}
;

Instr : Expr ';' 								{$$ = makeTree(YEXPR, 1, $1); }
| Bloc  										{$$ = $1; }
| RETURN ';' 									{$$ = NIL(Tree); }
| Selection AFF Expr ';' 						{$$ = makeTree(EAFF, 2, $1, $3); }
| IF Expr THEN Instr ELSE Instr 				{$$ = makeTree(YITE, 3, $2, $4, $6); }
;

LInstrOpt: LInstr 									{/*$$=makeTree(ILINSTO, 2, $1, $2);*/}
|  			 										{/* $$ = NIL(Tree); */}
;

LInstr : Instr LInstr 							{$$ = makeTree(LINSTR, 2, $1, $2);}
| Instr		 									{$$ = $1; }
;

BlocOpt: Bloc 										{	}
| 												{$$ = NIL(Tree);}
;

Bloc : '{' Contenu '}'    						{$$ = $2;}
;

BlocObj: '{' LDeclChampMethodOpt '}' 				{	}
;

LDeclChampMethodOpt: LDeclChampMethod 				{	}
| 												{$$ = NIL(Tree);}
;

LDeclChampMethod: LDeclChampMethod DeclChampMethod 	{		}
| DeclChampMethod 									{/*$$ = $1;*/	}
;

DeclChampMethod: DeclChamp 							{/*$$ = $1;*/	}
| DeclMethode 										{/*$$ = $1;*/	}
;

Contenu : LInstrOpt  							{$$ = $1; }
| LDeclChamp IS LInstr 							{$$ = makeTree(YCONT, 2, $1, $3);}
;

Envoi: Expr '.' MethodeC 							{/* $$ = makeTree(EEXPA, 2, $1, $3); */}
;

MethodeC: Id '(' LExprOpt ')' 						{	}
;

Selection: Expr '.' Id 							{$$ = makeTree(SEXPR, 2, $1, makeLeafStr(YID, $3)); }				/*{$$ = makeTree(SEXPR, 2, $1, makeLeafStr(YID, $3)); }*/
| Id 				   							{$$ = makeLeafStr(YID, $1); }
;
