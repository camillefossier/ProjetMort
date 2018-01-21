typedef struct Methode
{
	Classe *typeDeRetour;
	char *nom;
	Argument *larg;
	bool override;
	BLOC* Bloc;

} Methode;

typedef struct LMethode
{
	Methode* methode;
	LMethode* nextMethode;
} LMethode;

typedef struct Classe /*represente la meta classe*/
{

	char *nom;
	struct Classe *mereOpt;
	LMethode lmethodes;
	Methode *constructeur;
	LAttribut lattributs; /*peut etre null si pas de valeur par defaut*/

} Classe;


typedef struct LClasse
{
	Classe* classe;
	LClasse* next;
} LClasse;



enum typeAttribut 
{
	INTEGER, STRING, OTHER
};
typedef struct Attribut
{
	char *nom;
	Classe *type;
	typeAttribut typeA;


	union valeur 
	{
		int i;
		char[20] str;
		Objet *obj;
	}

} Attribut, Param, Champ;

typedef struct LAttribut
{
	Attribut* attribut;
	LAttribut* nextAtribut;
} LAttribut, LParam, LChamp;


typedef struct Objet
{
	Classe *classe;
	LAttribut *lattributs; /*attributs avec leur valeur courante*/

} Objet;


typedef struct ObjetIsole
{
	char *nom;
	Methode *lmethodes;
	LAttribut *lattributs;
} ObjetIsole;


typedef struct Argument
{
	Classe *type;
	char *nom;
} Argument;



/*ATTENTION A PARTIR DICI C LE ZBEUL**********************OK G COMPRIS JE GERE TKT*/
enum typeExpr {ID, CST, /*EXPR, */CAST, SELECT, INST, ENVOI, EXPROPE};
typedef struct Expression
{
	typeExpr type;
	union expr
	{
		Ident *id;
		Const cst;
		struct Expression *expr; /*utilité ?*/
		Cast *c;
		Selection *s;
		Instanciation *inst;
		Envoi *e;
		ExprOpe *eo;
	}
} Expression;

enum typeIdent {INTEGER, STRING}; /*et les methodes ?*/
typedef struct Ident
{
	char* nom;
	typeIdent type;
	union valeur 
	{
		char[20] str;
		int i;
	}

} Ident;

enum typeConst {INTEGER, STRING, VOIDUH};
typedef struct Const
{
	char* nom;
	typeConst type;
	union valeur
	{
		char[20] str;
		int i;
	}
} Const;

enum typeInstruction {EXPR, BLOC, RETURN, SELECT, ITE};
typedef struct Instruction
{
	typeInstruction type;

	union instr
	{
		Expression *expr;
		Bloc *bloc;
		Selection *select;
		struct {
			Expression *iteIf;
			Instruction *iteThen;
			Instruction *iteElse;
		} *ite;
	}

} Instruction;

typedef struct LInstruction
{
	Instruction* instruction;
	LInstruction* nextInstruction;
} LInstruction;


typedef struct Bloc
{
	DeclChamp *ldeclchamp;
	LInstruction lInstr;
}Bloc;

typedef struct Envoi{
	Expression *dst;/*probleme ambiguité*/
	Methode *methode;
	Expression *lExprOpt; 
} Envoi;

enum typeSelection {ID, THIS, SUPER, RESULT};
typedef struct Selection
{
	typeSelection type;
	Ident *ident;
} Selection;

typedef struct Programme
{
	LClasse lClasse;
	Bloc *bloc;
} Programme;

/*
typedef struct ClasseTete
{

} ClasseTete;
QUOI MAIS CEST JUSTE UN ID ALALALALALALALALALAALLA*/

enum typetypeC {INTC, STRINGC, VOIDC, IDENT};
typedef struct typeC
{
	typetypeC type;
	Classe *classe;
} typetypeC;

