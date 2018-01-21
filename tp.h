#include <stdlib.h>

#define TRUE 1
#define FALSE 0

typedef unsigned char bool;

/* deux macros pour eviter de faire des malloc en se trompant de type : on
 * s'oblige a expliciter le type d'objet a allouer et on récupere une
 * de type pointeur sur le type alloue.
 * Exemple: Allouer 3 objets de la structure Tree (voir ci-dessous)
 * TreeP p := NEW(3, Tree);
 *
 * De meme, sobloiger à mettre un type au NIL ci-dessous permet souvent
 * d'eviter de confondre entre des pointeurs de type differents.
 */
#define NEW(howmany, type) (type *) calloc((unsigned) howmany, sizeof(type))
#define NIL(type) (type *) 0

/* Etiquettes additionnelles pour les arbres de syntaxe abstraite.
 * Certains tokens servent directement d'etiquette. Attention ici a ne pas
 * donner des valeurs identiques a celles des tokens.
 *
 * Il y a surement des choses a recuperer en plus de ce que vous avez
 * produit pour le tp.
 */
#define NE  1
#define EQ  2
#define INF 3
#define INFE  4
#define SUP 5
#define SUPE  6


/* Codes d'erreurs. Cette liste n'est pas obligatoire ni limitative */
#define NO_ERROR  0
#define USAGE_ERROR 1
#define LEXICAL_ERROR 2
#define SYNTAX_ERROR    3
#define CONTEXT_ERROR 40  /* default value for this stage */
#define DECL_ERROR  41  /* scope problem */
#define TYPE_ERROR  42  /* type checking problem */
#define OTHER_CONTEXTUAL_ERROR  43 /* replace by something more precise */
#define EVAL_ERROR  50  /* dubious when writing a compiler ! */
#define UNEXPECTED  10O


/* Adapt as needed. Currently it is simply a list of names ! */
typedef struct _varDecl {
  char *name;
  struct _varDecl *next;
} VarDecl, *VarDeclP;


/* la structure d'un arbre (noeud ou feuille) */
typedef struct _Tree {
  short op;         /* etiquette de l'operateur courant */
  short nbChildren; /* nombre de sous-arbres */
  union {
    char *str;      /* valeur de la feuille si op = Id ou STR */
    int val;        /* valeur de la feuille si op = Cste */
    VarDeclP lvar;  /* ne pas utiliser tant qu'on n'en a pas besoin :-) */
    struct _Tree **children; /* tableau des sous-arbres */
  } u;
} Tree, *TreeP;


typedef union { 
  char *S;
  char C;
  int I;
  TreeP pT;
  VarDeclP pV; /* same comment as above */
} YYSTYPE;



typedef struct _Methode
{
  struct Classe *typeDeRetour;
  char *nom;
  struct Argument *larg;
  bool override;
  struct Bloc* Bloc;

}Methode, *MethodeP;

typedef struct _LMethode
{
  struct Methode* methode;
  struct LMethode* nextMethode;
}LMethode, *LMethodeP;

typedef struct _Classe /*represente la meta classe*/
{

  char *nom;
  struct Classe *mereOpt;
  struct LMethode *lmethodes;
  struct Methode *constructeur;
  struct LAttribut *lattributs; /*peut etre null si pas de valeur par defaut*/
} Classe, *ClasseP;


typedef struct _LClasse
{
  struct Classe* classe;
  struct LClasse* next;
} LClasse, *LClasseP;



enum typeAttribut 
{
  INTATTR, STRATTR, OTHER
};
typedef struct _Attribut
{
  char *nom;
  struct Classe *type;
  enum typeAttribut typeA;


  union valeurAttribut
  {
    int i;
    char *str;
    struct Objet *obj;
  } valeurAttribut;

} Attribut, Param, Champ, *AttributP, *ParamP, *ChampP;

typedef struct LAttribut
{
  struct Attribut* attribut;
  struct LAttribut* nextAtribut;
} LAttribut, LParam, LChamp;


typedef struct _Objet
{
  struct Classe *classe;
  struct LAttribut *lattributs; /*attributs avec leur valeur courante*/

} Objet, *ObjetP;


typedef struct _ObjetIsole
{
  char *nom;
  struct Methode *lmethodes;
  struct LAttribut *lattributs;
} ObjetIsole, *ObjetIsoleP;


typedef struct _Argument
{
  struct Classe *type;
  char *nom;
} Argument, *ArgumentP;



/*ATTENTION A PARTIR DICI C LE ZBEUL**********************OK G COMPRIS JE GERE TKT*/
enum typeExpr {ID, CST, /*EXPR, */CAST, SELECT, INST, ENVOI, EXPROPE};
typedef struct _Expression
{
  enum typeExpr type;
  union expr
  {
    struct Ident *id;
    struct Const *cst;
    struct Expression *expr; /*utilité ?*/
    struct Cast *c;
    struct Selection *s;
    struct Instanciation *inst;
    struct Envoi *e;
    struct ExprOpe *eo;
  } expr;
} Expression, *ExpressionP;

enum typeIdent {INTIDENT, STRIDENT}; /*et les methodes ?*/
typedef struct _Ident
{
  char* nom;
  enum typeIdent type;
  union valeurIdent 
  {
    char *str;
    int i;
  } valeurIdent;

} Ident, *IdentP;

enum typeConst {INTCONST, STRCONST, VOIDCONST};
typedef struct _Const
{
  char* nom;
  enum typeConst type;
  union valeurConst
  {
    char* str;
    int i;
  } valeurConst;
} Const, *ConstP;

enum typeInstruction {EXPRINSTR, BLOCINSTR, RETURNINSTR, SELECTINSTR, ITEINSTR};
typedef struct _Instruction
{
  enum typeInstruction type;

  union instr
  {
    struct Expression *expr;
    struct Bloc *bloc;
    struct Selection *select;
    struct {
      struct Expression *iteIf;
      struct Instruction *iteThen;
      struct Instruction *iteElse;
    } *ite;
  } instr;

} Instruction, *InstructionP;

typedef struct _LInstruction
{
  struct Instruction* instruction;
  struct LInstruction* nextInstruction;
} LInstruction, *LInstructionP;


typedef struct _Bloc
{
  struct DeclChamp *ldeclchamp;
  struct LInstruction *lInstr;
}Bloc, *BlocP;

typedef struct _Envoi{
  struct Expression *dst;/*probleme ambiguité*/
  struct Methode *methode;
  struct Expression *lExprOpt; 
} Envoi, *EnvoiP;

enum typeSelection {IDSELEC, THISSELEC, SUPERSELEC, RESULTSELEC};
typedef struct _Selection
{
  enum typeSelection type;
  struct Ident *ident;
} Selection, *SelectionP;

typedef struct _Programme
{
  struct LClasse *lClasse;
  struct Bloc *bloc;
} Programme, *ProgrammeP;

/*
typedef struct ClasseTete
{

} ClasseTete;
QUOI MAIS CEST JUSTE UN ID ALALALALALALALALALAALLA*/

enum typetypeC {INTTYPEC, STRTYPEC, VOIDTYPEC, IDTYPEC};
typedef struct _typeC
{
  enum typetypeC type;
  struct Classe *classe;
}typeC, *typeCP;



Classe makeClasse(char* nom, struct Classe mereOpt, LMethode lmethodes, Methode constructeur, LAttribut lattributs);
Methode makeMethode(Classe typeDeRetour, char nom, Argument larg, bool override, Bloc bloc);


#define YYSTYPE YYSTYPE
#define YYERROR_VERBOSE 1