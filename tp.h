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
#define YPROG 7
#define YCONT 8
#define LDECLC 9
#define YDECLC 10
#define LINSTR 11
#define EAFF 12
#define YITE 13
#define ECAST 16
#define YSELEC 17
#define EINST 18
#define SELEXPR 21
#define YEXPR 23
#define YLEXPR 24
#define YEXT 25
#define EENVOI 26
#define METHOD 27
#define YLCLASS 28
#define YCLASS 29
#define LDECLMETH 30
#define DMETHODE 31
#define YLPARAM 32
#define YPARAM 33
#define YOBJ 34

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

enum typeAttribut 
{
  INTATTR, STRATTR, OTHER
};
typedef struct _Attribut
{
  char *nom;
  struct _Classe *type;
  enum typeAttribut typeA;


  union valeurAttribut
  {
    int i;
    char *str;
    struct Objet *obj;
  } valeurAttribut;

} Attribut, Param, Champ, *AttributP, *ParamP, *ChampP;

/* Adapt as needed. Currently it is simply a list of names ! */
typedef struct _varDecl {
  char *name;
  struct _varDecl *next;
  struct _Attribut *attr;
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

typedef struct _Classe /*represente la meta classe*/
{

  char *nom;
  struct _Classe* mereOpt;
  struct LMethode *lmethodes;
  struct Methode *constructeur;
  struct LAttribut *lattributs; /*peut etre null si pas de valeur par defaut*/
} Classe, *ClasseP;

typedef struct _LClasse
{
  struct _Classe *classe;
  struct _LClasse *next;
}LClasse, *LClasseP;

typedef struct _Methode
{
  struct _Classe *typeDeRetour;
  char *nom;
  struct LParam *larg;
  bool override;
  struct Bloc* Bloc;

}Methode, *MethodeP;

typedef struct _LMethode
{
  struct Methode* methode;
  struct LMethode* next;
}LMethode, *LMethodeP;


typedef struct LAttribut
{
  struct _Attribut* attribut;
  struct LAttribut* next;
} LAttribut, LParam, LChamp, *LAttributP, *LParamP, *LChampP;

typedef struct _BlocObj
{
  struct LAttributs* lattributs;
  struct LMethode* lmethodes;
}BlocObj, *BlocObjP;

typedef struct _Bloc
{
  struct DeclChamp *ldeclchamp;
  struct LInstruction *lInstr;
}Bloc, *BlocP;


typedef struct _Objet
{
  struct _Classe *classe;
  struct LAttribut *lattributs; /*attributs avec leur valeur courante*/

} Objet, *ObjetP;


typedef struct _ObjetIsole
{
  char *nom;
  BlocObjP bloc;
} ObjetIsole, *ObjetIsoleP;

/*
typedef struct _Argument
{
  struct Classe *type;
  char *nom;
} Argument, *ArgumentP;
*/


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

/*typedef struct _ValVar
{

} ValVar, **/
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

typedef union 
{ 
  ClasseP pC;
  char *S;
  char C;
  int I;
  TreeP pT;
  LParamP pV; /* same comment as above */
  LAttributP pVD;
  /*BlocP pB;*/
  BlocObjP pBO;
  ObjetIsoleP pOI;
  ProgrammeP pP;
} YYSTYPE;


ClasseP makeClasse(char* nom);

ObjetIsoleP makeObjetIsole(char *nom, BlocObjP bloc);

TreeP makeTree(short op, int nbChildren, ...);
TreeP makeLeafStr(short op, char *str);
TreeP makeLeafInt(short op, int val);
TreeP makeLeafLVar(short op, VarDeclP lvar);
TreeP getChild(TreeP tree, int rank);
void afficherProgramme(TreeP tree) ;
void makeStructures(TreeP lclasse);
char* checkExpr(TreeP tree, ClasseP classes, VarDeclP env);
bool checkClassDefine(ClasseP env_classe, char* nom);
void transmettreEnv(TreeP tree);
bool checkPortee(VarDeclP lvar, char* nom);
void compile(TreeP lclasse, TreeP main);
void makeClassesParDefaut();
void addClasse(ClasseP classe);

ParamP makeParam(char *nom, ClasseP type);
void initClasse(ClasseP classe, TreeP lparam, TreeP mere, TreeP constructeur, TreeP methode);


#define YYSTYPE YYSTYPE
#define YYERROR_VERBOSE 1