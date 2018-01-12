/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */
%token IS CLASS VAR EXTENDS DEF OVERRIDE RETURN AS IF THEN ELSE AFF NEWC VOIDC INTC STRINGC THIS SUPER RESULT ADD SUB MUL DIV CONCAT
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
			TypeCOpt Expr ExprLOpt LExpr
			ExprOperateur Instr InstrLOpt 
			LInstr Bloc 
			Envoi Selection
%type <pC> 	Class classTete classLOpt
%type <pM> 	DeclMethodeLOpt DeclMethode
%type <pV> 	DeclChampLOpt LDeclChamp DeclChamp Param paramLOpt
			LParam ValVar
%type <I> 	OverrideOpt

%{
#include "tp.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(char *);
%}

%%
Prog : classLOpt Bloc
;

classTete: CLASS Classname {/* $$ = makeClass($2); */}
;

classLOpt: Class classLOpt {/* $1->nextClass = $2; $$ = $1 ;*/}
| 	{/*$$ = NIL(Class)*/}
;

Class: Objet {} 
| classTete '(' paramLOpt ')' ExtendsOpt BlocOpt IS BlocObj 
								 {/* initClasse($1, $3, $5, $6, $9, $10); $$ = $1; */}
;

Objet: OBJECT Classname IS BlocObj {}
;

paramLOpt: LParam {/*	$$ = $1; */}
|				  {/* $$ = NIL(VarDecl); */}
;

LParam: Param ',' LParam {/* $1->nextParam = $3; $$=$1; */}
| Param            	     {/* $$ = $1; */}
;

Param: VAR Id ':' TypeC ValVar {/* $$ = makeParam($1,$3,NIL(Tree),0); */}
| Id ':' TypeC ValVar {/* $$ = makeParam($1,$3,NIL(Tree),0); */}
;

ValVar: AFF Expr {}
| {}
;

DeclChampLOpt: DeclChamp DeclChampLOpt {/* $1->nextParam = $2; $$ = $1;  */}
| 			 {/* $$ = NIL(VarDecl); */}
;

LDeclChamp: DeclChamp LDeclChamp {/* $1->nextParam = $3; $$ = $1;*/}
| DeclChamp {/* $$ = $1; */}
;

DeclChamp: VAR Id ':' TypeC AffOpt ';' {/* $$ = makeParam($2, $4, $5, 2); */}
;

TypeC: INTC {/* $$ = makeLeafStr(APPC, "Integer"); */}    //INTC STRINGC ET VOIDC peuvent êtres traités dans Classname en les mettant dans le même environnement (qui sera rentré à la main à la compilation)
| STRINGC {/* $$ = makeLeafStr(APPC, "String"); */}
| VOIDC {/*TODO; */}
| Classname {/* $$ = makeLeafStr(APPC, $1); */}
;

AffOpt: AFF Expr {/* $$ = makeTree(AEO, 2, NIL(Tree), $2); */}
|				 {/* $$ = NIL(Tree); */}
;

DeclMethodeLOpt: DeclMethode ';' DeclMethodeLOpt {/* $1->nextMethode = $2; $$ = $1; */}
|  				{/* $$ = NIL(Methode); */}
;

DeclMethode : OverrideOpt DEF Id '(' paramLOpt ')' ':' TypeC AFF Expr {/* $$ = makeMethode($1, $3, $5, $8, $10, 1); */}
| OverrideOpt DEF Id '(' paramLOpt ')' TypeCOpt IS Bloc {}
/*TODO Bloc { $$ = makeMethode($1, $3, $5, $7, $9, 1); }*/

;

TypeCOpt: ':' TypeC {/* $$ = makeTree(EAPPC, 2, NIL(Tree), $2); */}
| {/* $$ = NIL(Tree); */}
;

OverrideOpt: OVERRIDE {/* $$ = TRUE; */}
|     				  {/* $$ = FALSE; */}
;

ExtendsOpt: EXTENDS Classname '(' ExprLOpt ')'  {/* TreeP nC = makeLeafStr(APPC, $2); $$ = makeTree(EXTO, 2, nC, $4); */}
|					{/* $$ = NIL(Tree); */}
;

Expr: /*Id*/ /*{$$ = makeLeafStr(ECHAIN, $1); }*/
 Cste {/* $$ = makeLeafInt(ECONST, $1); */}
| '(' Expr ')'  {/* $$ = makeTree(EPEXP, 2, NIL(Tree), $2); */}
| '(' TypeC Expr ')' 	{/* $$ = makeTree(ECAST, 2, NIL(Tree), $1); */}	//Correspond à un Cast
| Selection    {/* $$ = makeTree(ESEL, 2, NIL(Tree), $1); */}
| NEWC Classname '(' ExprLOpt ')' {/* $$ = makeTree(EINSTA, 2, NIL(Tree), $1); */}   //Correspond à une Instanciation
| Envoi  {/* $$ = makeTree(EENV, 2, NIL(Tree), $1); */}
| ExprOperateur {/* $$ = makeTree(EOPER, 2, NIL(Tree), $1); */}
| TypeC {}
;

ExprLOpt: LExpr {/* $$ = makeTree(EEXPO, 2, NIL(Tree), $1); */}
| 				{/* $$ = NIL(Tree); */}
;

LExpr: Expr ',' LExpr {/* $$ = makeTree(LEXP, 2, $1, $3); */}
| Expr 			  {/* $$ = makeTree(EEXP, 2, NIL(Tree), $1); */}
;

ExprOperateur: Expr ADD Expr {/* $$ = makeTree(EADD, 2, $1, $3); */}
| Expr SUB Expr {/* $$ = makeTree(EMINUS, 2, $1, $3); */}
| Expr MUL Expr {/* $$ = makeTree(EMUL, 2, $1, $3); */}
| Expr DIV Expr  {/* $$ = makeTree(EQUO, 2, $1, $3); */}
| SUB Expr %prec UNARY {/* $$ = makeTree(EMINUS, 2, makeLeafInt(CONST, 0), $2); */}
| ADD Expr %prec UNARY {/* $$ = makeTree(EADD, 2, makeLeafInt(CONST, 0), $2); */}      
| Expr CONCAT Expr {/* $$ = makeTree(EAND, 2, $1, $3); */}
| Expr RelOp Expr 	{/* $$ = makeTree($2, 2, $1, $3); */}
;

Instr : Expr ';' {/* $$ = makeTree(EEXP, 2, NIL(Tree), $1); */}
| Bloc  {/*$$ = makeTree(EBLOC, 2, NIL(Tree), $1); */}
| RETURN ';' {/* $$ = makeLeafStr(ERET, NIL(char)); */}
| Selection AFF Expr ';' {/* $$ = makeTree(EAFF, 2, $1, $3); */}
| IF Expr THEN Instr ELSE Instr {/* $$ = makeTree(ITE, 3, $2, $4, $6); */}
;

InstrLOpt: LInstr {/*$$=makeTree(ILINSTO, 2, $1, $2);*/}
|  			 {/* $$ = NIL(Tree); */}
;

LInstr : Instr LInstr {/*$$ = makeTree(LINST, 2, $1, $2);*/}
| Instr		 {/*$$ = makeTree(EINST, 2, $1, NIL(Tree)); */}
;

BlocOpt: Bloc {}
| {}
;

Bloc : '{' Contenu '}'    {/* $$ = makeTree(EIB, 2, NIL(Tree), $2); */}
;

BlocObj: '{' LDeclChampMethodOpt '}' {}
;

LDeclChampMethodOpt: LDeclChampMethod {}
| {}
;

LDeclChampMethod: LDeclChampMethod DeclChampMethod {}
| DeclChampMethod {}
;

DeclChampMethod: DeclChamp {}
| DeclMethode {}
;

Contenu : InstrLOpt  {/* $$ = makeTree(LINSTO, 2, NIL(Tree), $1); */}
| LDeclChamp IS LInstr {/* $$ = makeTree(EIS, 2, $1, $3); */}
;

Envoi: Expr '.' MethodeC {/* $$ = makeTree(EEXPA, 2, $1, $3); */}
;

MethodeC: Id '(' ExprLOpt ')' {}
;

Selection: Expr '.' Id {/* $$ = makeTree(EEXPI, 2, $1, makeLeafStr(IDVAR, $3)); */}
| Id 				   {/* $$ = makeLeafStr(IDVAR, $1); */}
| THIS 				   {/* $$ = makeLeafInt(ETHIS, 0); */}
| SUPER				   {/* $$ = makeLeafInt(ESUP, 0); */}
| RESULT 			   {/* $$ = makeLeafInt(ERES, 0); */} //RESULT peut être traité dans Id en le mettant dans le même environnement (qui sera rentré à la main à la compilation)
;




