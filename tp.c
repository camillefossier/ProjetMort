#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"
#include <assert.h>

extern int yyparse();
extern int yylineno;

/* Peut servir a controloer le niveau de 'verbosite'.
 * Par defaut, n'imprime que le resultat et les messages d'erreur
 */
bool verbose = FALSE;

/* Peut servir a controler la generation de code. Par defaut, on produit le code
 * On pourrait avoir un flag similaire pour s'arreter avant les verifications
 * contextuelles (certaines ou toutes...)
 */
bool noCode = FALSE;

/* Pour controler la pose de points d'arret ou pas dans le code produit */
bool debug = FALSE;

/* code d'erreur a retourner */
int errorCode = NO_ERROR; /* defini dans tp.h */

FILE *out; /* fichier de sortie pour le code engendre */

/* VARIABLE GLOBALE QUI CONTIENT LA LISTE DES STRUCTURES CLASSES DU PROGRAMME issou de la rbre*/
LClasseP lclasse = NIL(LClasse);

int main(int argc, char **argv) {
  int fi;
  int i, res;

  out = stdout;
  for(i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'd': case 'D':
	debug = TRUE; continue;
      case 'v': case 'V':
	verbose = TRUE; continue;
      case 'e': case 'E':
	noCode = TRUE; continue;
      case '?': case 'h': case 'H':
	fprintf(stderr, "Appel: tp -v -e -d -o file.out programme.txt\n");
	exit(USAGE_ERROR);
      case'o':
	  if ((out= fopen(argv[++i], "w")) == NULL) {
	    fprintf(stderr, "erreur: Cannot open %s\n", argv[i]);
	    exit(USAGE_ERROR);
	  }
	break;
      default:
	fprintf(stderr, "Option inconnue: %c\n", argv[i][1]);
	exit(USAGE_ERROR);
      }
    } else break;
  }

  if (i == argc) {
    fprintf(stderr, "Fichier programme manquant\n");
    exit(USAGE_ERROR);
  }

  if ((fi = open(argv[i++], O_RDONLY)) == -1) {
    fprintf(stderr, "erreur: Cannot open %s\n", argv[i-1]);
    exit(USAGE_ERROR);
  }

  /* redirige l'entree standard sur le fichier... */
  close(0); dup(fi); close(fi);

  res = yyparse();
  /* si yyparse renvoie 0, le programme en entree etait syntaxiquement correct.
   * Le plus simple est que les verifications contextuelles et la generation
   * de copde soient lancees par les actions associees a la regle de grammaire
   * pour l'axiome. Dans ce cas, quand yyparse renvoie sa valeur on n'a plus
   * rien a faire, sauf fermer les fichiers qui doivent l'etre.
   * Si yyparse renvoie autre chose que 0 c'est que le programme avait une
   * erreur lexicale ou syntaxique
   */
  if (out != NIL(FILE) && out != stdout) fclose(out);
  return res ? SYNTAX_ERROR : errorCode;
}


void setError(int code) {
  errorCode = code;
  if (code != NO_ERROR) {
    noCode = TRUE;
    /* la ligne suivante peut servir a "planter" volontairement le programme
     * des qu'une de vos fonctions detectent une erreur et appelle setError.
     * Si vous executez le rpogramme sous le debuggeur vous aurez donc la main
     * et pourrez examiner la pile d'execution.
     */
    /*  abort(); */
  }
}


/* yyerror:  fonction importee par Bison et a fournir explicitement. Elle
 * est appelee quand Bison detecte une erreur syntaxique.
 * Ici on se contente d'un message a minima.
 */
void yyerror(char *ignore) {
  printf("erreur de syntaxe: Ligne %d\n", yylineno);
  setError(SYNTAX_ERROR);
}



/* ****** Fonctions pour la construction d'AST   ********************
 *
 * Ajoutez vos propres constructeurs, si besoin
 *
 */

/* Tronc commun pour la construction d'arbre. Normalement on ne l'appelle
 * pas directement. Elle ne fait qu'allouer, sans remplir les champs
 */
TreeP makeNode(int nbChildren, short op) {
  TreeP tree = NEW(1, Tree);
  tree->op = op;
  tree->nbChildren = nbChildren;
  tree->u.children = nbChildren > 0 ? NEW(nbChildren, TreeP) : NIL(TreeP);
  return(tree);
}


/* Construction d'un arbre a nbChildren branches, passees en parametres.
 * Pour comprendre en detail (si necessaire), regardez un tutorial sur
 * comment on passe un nombre variable d'arguments à une fonction et
 * comment on recupere chacun de ces arguments.
 * cf va_list, va_start, va_arg et va_end.
 * makeTree prend donc toujours au moins 2 arguments
 */
TreeP makeTree(short op, int nbChildren, ...) {
  va_list args;
  int i;
  TreeP tree = makeNode(nbChildren, op);
  va_start(args, nbChildren);
  for (i = 0; i < nbChildren; i++) {
    tree->u.children[i] = va_arg(args, TreeP);
  }
  va_end(args);
  return(tree);
}


/* Retourne le rank-ieme fils d'un arbre (de 0 a n-1) */
TreeP getChild(TreeP tree, int rank) {
  if (tree->nbChildren < rank -1) {
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort(); /* plante le programme en cas de rang incorrect */
  }
  return tree->u.children[rank];
}


void setChild(TreeP tree, int rank, TreeP arg) {
  if (tree->nbChildren < rank -1) {
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort(); /* plante le programme en cas de rang incorrect */
  }
  tree->u.children[rank] = arg;
}


/* Constructeur de feuille dont la valeur est une chaine de caracteres */
TreeP makeLeafStr(short op, char *str) {
  TreeP tree = makeNode(0, op);
  tree->u.str = str;
  return tree;
}


/* Constructeur de feuille dont la valeur est un entier */
TreeP makeLeafInt(short op, int val) {
  TreeP tree = makeNode(0, op);
  tree->u.val = val;
  return(tree);
}

/* Constructeur de feuille dont la valeur est une declaration */
TreeP makeLeafLVar(short op, VarDeclP lvar) {
  TreeP tree = makeNode(0, op);
  tree->u.lvar = lvar;
  return(tree);
}


ObjetIsoleP makeObjetIsole(char* nom, BlocObjP bloc)
{
  ObjetIsoleP objet = NEW(1, ObjetIsole);
  objet->nom = nom;
  objet->bloc = bloc;

  printf("L'objet cree est : %s\n", objet->nom);

  
  return objet;
}

void compile(TreeP lclasse, TreeP main)
{
  makeStructures(lclasse);

  /*blabla*/
}

ClasseP getClassePointer(char *nomClasse)
{
  LClasseP cur = lclasse;
  while (cur != NIL(LClasse))
  {
    if (strcmp(cur->classe->nom, nomClasse) == 0)
    {
      return cur->classe;
    }
  }

  return NIL(Classe);
}

void makeStructures(TreeP lclasse)
{
   TreeP courant = lclasse;

   /*makeClassesParDefaut();*/

  while(courant != NIL(Tree))
  {
    /*makeClasse(getChild(courant, 0));*/
    courant = getChild(courant, 1);
  }
}


ClasseP makeClasse(char *nom)
{
  ClasseP classe = NEW(1, Classe);
  classe->nom = nom;

  addClasse(classe);
  printf("La classe cree est : %s\n", classe->nom);

  return classe;
}

void addClasse(ClasseP classe){

    LClasseP newClasse = NEW(1, LClasse);

    newClasse->classe = classe;
    newClasse->next = NIL(LClasse);  
    if(lclasse == NIL(LClasse))
    {
        lclasse = newClasse;
    }

    else
    {
        LClasseP current = lclasse;
        while (TRUE) { 
            if(current->next == NULL)
            {
                current->next = newClasse;
                break; 
            }
            current = current->next;
        };
    }
}

ParamP makeParam(char *nom, ClasseP type){

  ParamP p = NEW(1, Param);
  p->nom = nom;
  p->type = type;

  return p;
}
/*
Methode makeMethode(TreeP override, TreeP nom, TreeP)
{
  Methode methode = NEW(1, Methode);

  methode->typeDeRetour = typeDeRetour;
  methode->nom = nom;
  methode->larg = larg;

  methode.override = override;
  methode->bloc = bloc;

  return (methode);
}*/


void makeClassesParDefaut()
{
  ClasseP integer = makeClasse("Integer");
  ClasseP string = makeClasse("String");
  ClasseP voidC = makeClasse("Void");


  LParamP lp = NEW(1,LParam);
  ParamP param = makeParam("val", getClassePointer("Integer"));
  lp->next = NIL(LParam);
  lp->attribut = param;

  MethodeP constrInt = NEW(1, Methode);
  constrInt->typeDeRetour = integer;
  constrInt->override = FALSE;
  constrInt->nom = "Integer";
  constrInt->larg = lp;

  /*initClasse(integer,  , NIL(Classe), NIL(Bloc), NIL(Bloc));*/
 /* initClasse(integer, )*/
}

void initClasse(ClasseP classe, TreeP lparam, TreeP mere, TreeP constructeur, TreeP methode)
{

}

void afficherProgramme(TreeP tree) 
{
  if(tree != NIL(Tree)) 
  {
    switch (tree->op) {
    case YPROG :
      printf(" PROG \n");
      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      break;
    case YCONT :
      printf("\n {CONT");
      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      printf("\n}");
      break;
    case LDECLC :
      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      break;
    case YDECLC :
      printf("\nDECL var ");
      afficherProgramme(tree->u.children[0]);

      printf(": ");
      afficherProgramme(tree->u.children[1]);

      printf(" := ");
      afficherProgramme(tree->u.children[2]);
      printf("; \n");
      break;
    case LINSTR :
      printf(" LINSTR ");
      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      break;

    case YOBJ :
      printf("\n OBJ object ");
      afficherProgramme(tree->u.children[0]);
      printf(" is ");
      afficherProgramme(tree->u.children[1]);
      break;

    case YLCLASS :
      
      printf("\n[LCLASS ");  

      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      printf("]\n");
      break;

    case YCLASS :

      printf("\n[ CLASS class ");
      afficherProgramme(tree->u.children[0]);
      printf("(");

      afficherProgramme(tree->u.children[1]);
      printf(") ");
      afficherProgramme(tree->u.children[2]);
      afficherProgramme(tree->u.children[3]);
      printf("\nis ");

      afficherProgramme(tree->u.children[4]);
      printf("]\n");

      break;

    case Chaine :
      printf("%s", tree->u.str);
      break;

    case STRINGC :
      printf("String");
      break;

    case YITE :
      printf("\nITE if ");
        afficherProgramme(tree->u.children[0]);
      printf("{");
        afficherProgramme(tree->u.children[1]);
      printf("} else ");
      printf("{");
        afficherProgramme(tree->u.children[2]);
      printf(" }\n");

      break;

    case EAFF :
      afficherProgramme(tree->u.children[0]);
      printf(" := ");
      afficherProgramme(tree->u.children[1]);
      printf(";\n");
      break;

    case ECAST :
      printf("CAST (");
      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;


    case YEXPR :
      printf("EXPR (");
      if(tree->nbChildren <= 1){
        afficherProgramme(tree->u.children[0]);
      }
      else{
        afficherProgramme(tree->u.children[0]);
        afficherProgramme(tree->u.children[1]);
      }
      printf(")");

      break;

    case EINST :
      printf("new ");
      afficherProgramme(tree->u.children[0]);
      printf("(");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case INTC :
      printf("Integer");
      break;

    case SELEXPR :
      afficherProgramme(tree->u.children[0]);
      printf(".");
      afficherProgramme(tree->u.children[1]);

      break;

    case Id :
      printf("%s", tree->u.str);  
      break;

    case Cste :
      printf("%d", tree->u.val);  
      break;

    case Classname :
      printf("%s", tree->u.str);  
      break;

    case YEXT :
      printf("extends ");
      afficherProgramme(tree->u.children[0]);
      printf("(");
      afficherProgramme(tree->u.children[1]);
      printf(")\n");
      break;

    case YLEXPR :
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(", ");
      afficherProgramme(tree->u.children[1]);
      printf(")\n");
      break;


    case EENVOI :
      afficherProgramme(tree->u.children[0]);
      printf(".");  
      afficherProgramme(tree->u.children[1]);

      break;

    case METHOD :
      afficherProgramme(tree->u.children[0]);
      printf("(");
      afficherProgramme(tree->u.children[1]);
      printf(")\n");
      break;

    case LDECLMETH :
      printf("\n[LDECLMETH");
      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      printf("]\n");
      break ;

    case DMETHODE :
      printf("\nDMETHODE def ");
      afficherProgramme(tree->u.children[0]); 
      afficherProgramme(tree->u.children[1]);
      printf("(");
      afficherProgramme(tree->u.children[2]); 
      printf(") :");

      afficherProgramme(tree->u.children[3]);
      printf(" := ");

      afficherProgramme(tree->u.children[4]);
      printf("\n");

      break;

    case YLPARAM :
      printf("\nYLPARAM");
      afficherProgramme(tree->u.children[0]);
      afficherProgramme(tree->u.children[1]);
      printf("]\n");

      break;
    case YPARAM :
      printf("\nYPARAM ");
      afficherProgramme(tree->u.children[0]);
      printf(" : ");
      afficherProgramme(tree->u.children[1]);
      afficherProgramme(tree->u.children[2]);
      printf("\n");
      break;


    case ADD :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" + ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;
    case SUB :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" - ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;
    case MUL :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" * ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case DIV :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" / ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case CONCAT :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" & ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case EQ :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(") = (");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case NE :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" <> ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case INF :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" < ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case INFE :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" <= ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case SUP :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" > ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    case SUPE :
      assert(tree->nbChildren == 2);
      printf("(");
      afficherProgramme(tree->u.children[0]);
      printf(" >= ");
      afficherProgramme(tree->u.children[1]);
      printf(")");
      break;

    default :
      printf("Bonjour Monsieur Voisin, si ce message s'affiche, c'est qu'il existe un cas inconnu dans la construction de l'arbre. Veuillez nous en excuser. Cordialement.");
      break;
  }
  }

  
}

/*
ParamP makeParam(char* nom, typeCP type,TreeP val)
{
  ParamP param = NEW(1,Param);
  param->nom = nom;
  param->typeAttribut = 

  return (param);
}
*/