typedef struct Methode
{
	struct Classe *typeDeRetour;
	char *nom;
	struct Argument *larg;
	bool override;
	struct Bloc* Bloc;

} Methode;

typedef struct LMethode
{
	struct Methode* methode;
	struct LMethode* nextMethode;
} LMethode;

typedef struct Classe /*represente la meta classe*/
{

	char *nom;
	struct Classe *mereOpt;
	struct LMethode lmethodes;
	struct Methode *constructeur;
	struct LAttribut *lattributs; /*peut etre null si pas de valeur par defaut*/
} Classe;


typedef struct LClasse
{
	struct Classe* classe;
	struct LClasse* next;
} LClasse;



enum typeAttribut 
{
	INTATTR, STRATTR, OTHER
};
typedef struct Attribut
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

} Attribut, Param, Champ;

typedef struct LAttribut
{
	struct Attribut* attribut;
	struct LAttribut* nextAtribut;
} LAttribut, LParam, LChamp;


typedef struct Objet
{
	struct Classe *classe;
	struct LAttribut *lattributs; /*attributs avec leur valeur courante*/

} Objet;


typedef struct ObjetIsole
{
	char *nom;
	struct Methode *lmethodes;
	struct LAttribut *lattributs;
} ObjetIsole;


typedef struct Argument
{
	struct Classe *type;
	char *nom;
} Argument;



/*ATTENTION A PARTIR DICI C LE ZBEUL**********************OK G COMPRIS JE GERE TKT*/
enum typeExpr {ID, CST, /*EXPR, */CAST, SELECT, INST, ENVOI, EXPROPE};
typedef struct Expression
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
} Expression;

enum typeIdent {INTIDENT, STRIDENT}; /*et les methodes ?*/
typedef struct Ident
{
	char* nom;
	enum typeIdent type;
	union valeurIdent 
	{
		char *str;
		int i;
	} valeurIdent;

} Ident;

enum typeConst {INTCONST, STRCONST, VOIDUH};
typedef struct Const
{
	char* nom;
	enum typeConst type;
	union valeurConst
	{
		char* str;
		int i;
	} valeurConst;
} Const;

enum typeInstruction {EXPR, BLOC, RETURN, SELECT, ITE};
typedef struct Instruction
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

} Instruction;

typedef struct LInstruction
{
	struct Instruction* instruction;
	struct LInstruction* nextInstruction;
} LInstruction;


typedef struct Bloc
{
	struct DeclChamp *ldeclchamp;
	struct LInstruction lInstr;
}Bloc;

typedef struct Envoi{
	struct Expression *dst;/*probleme ambiguité*/
	struct Methode *methode;
	struct Expression *lExprOpt; 
} Envoi;

enum typeSelection {ID, THIS, SUPER, RESULT};
typedef struct Selection
{
	enum typeSelection type;
	struct Ident *ident;
} Selection;

typedef struct Programme
{
	struct LClasse lClasse;
	struct Bloc *bloc;
} Programme;

/*
typedef struct ClasseTete
{

} ClasseTete;
QUOI MAIS CEST JUSTE UN ID ALALALALALALALALALAALLA*/

enum typetypeC {INTC, STRINGC, VOIDC, IDENT};
typedef struct typeC
{
	enum typetypeC type;
	struct Classe *classe;
} typetypeC;

