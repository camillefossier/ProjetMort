#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"

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

ClasseP makeClasse(char* nom, LParamP lparam, ClasseP extendsOpt, BlocP blocOpt, BlocObjP blocObj)
{
  ClasseP classe = NEW(1, Classe);
  classe->nom = nom;

  printf("La classe cree est : %s\n", classe->nom);


  classe->nextClasse = NIL(Classe);
  return classe;
}


ObjetIsoleP makeObjetIsole(char* nom, BlocObjP bloc)
{
  ObjetIsoleP objet = NEW(1, ObjetIsole);
  objet->nom = nom;
  objet->bloc = bloc;

  printf("L'objet cree est : %s\n", objet->nom);

  
  return objet;
}
void printTree(TreeP tree) 
{
  if(tree != NIL(Tree)) 
  {
    switch (tree->op) {
    case YPROG :
      printf(" PROG \n");
      printTree(tree->u.children[0]);
      printTree(tree->u.children[1]);
      break;
    case YCONT :
      printf(" CONT \n");
      printTree(tree->u.children[0]);
      printTree(tree->u.children[1]);
      break;
    case LDECLC :
      printf(" LDECLC \n");
      printTree(tree->u.children[0]);
      printTree(tree->u.children[1]);
      break;
    case YDECLC :
      printf(" DECL \n");
      printTree(tree->u.children[0]);
      printTree(tree->u.children[1]);
      printTree(tree->u.children[2]);
      break;
    case LINSTR :
      printf(" LINSTR \n");
      printTree(tree->u.children[0]);
      printTree(tree->u.children[1]);
      break;
    case YINSTR :
      printf(" INSTR \n");
      break;
    case STRINGC :
      printf("%s", tree->u.str);
      break;
    case YITE :
      printf(" ITE \n");
      break;
    case EAFF :
      printf(" EAFF \n");
      break;
    case ECONST :
      printf(" ##########ECONST : ");
      printf("%d", tree->u.val);
      break;
    case ECAST :
      printf(" ECAST \n");
      break;


    case YEXPR :
    printf(" YEXPR \n");
      if(tree->nbChildren <= 1){
        printTree(tree->u.children[0]);
      }
      else{
        printf("Nb children = 2");
        printTree(tree->u.children[0]);
        printTree(tree->u.children[1]);
      }
      break;


    case EINST :
      printf(" EINST \n");
      break;
    case TINTC :
      printf(" TINTC \n");
      break;
    case TSTRINGC :
      printf(" TSTRINGC \n");
      break;
    case SEXPR :

      printf(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@SEXPR : ");
      printTree(tree->u.children[0]);
      printf(" . ");
      printTree(tree->u.children[1]);

      break;
    case YID :
      printf(" YID \n");  /*wow*/
      break;

    case YEXT :
      printf(" #######----------------------------############YEXT \n");   /*non plus je crois)*/
      break;

    case YLEXPR :
      printf(" ################ YLEXPR \n");  /*Ca ne s'affiche pas*/
      break;

    /*case Id :
      printf("%s", tree->u.lvar->nom);
      break;
    case ADD :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" + ");
      printTree(tree->u.children[1]);
      printf(")");
      break;
    case SUB :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" - ");
      printTree(tree->u.children[1]);
      printf(")");
      break;
    case MUL :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" * ");
      printTree(tree->u.children[1]);
      printf(")");
      break;
    case DIV :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" / ");
      printTree(tree->u.children[1]);
      printf(")");
      break;
    case CONCAT :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" & ");
      printTree(tree->u.children[1]);
      printf(")");
      break;
    case EQ :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(") = (");
      printTree(tree->u.children[1]);
      printf(")");
    case NE :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" <> ");
      printTree(tree->u.children[1]);
      printf(")");
    case INF :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" < ");
      printTree(tree->u.children[1]);
      printf(")");
    case INFE :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" <= ");
      printTree(tree->u.children[1]);
      printf(")");
    case SUP :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" > ");
      printTree(tree->u.children[1]);
      printf(")");
    case SUPE :
      assert(tree->nbChildren == 2);
      printf("(");
      printTree(tree->u.children[0]);
      printf(" >= ");
      printTree(tree->u.children[1]);
      printf(")");*/
    default :
      printf(" unknown ");
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
/*Methode makeMethode(Classe typeDeRetour, char nom, Argument larg, bool override, Bloc bloc)
{
  Methode methode = NEW(1, Methode);

  methode->typeDeRetour = typeDeRetour;
  methode->nom = nom;
  methode->larg = larg;

  methode.override = override;
  methode->bloc = bloc;

  return (methode);
}
*/