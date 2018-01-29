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

/* VARIABLE GLOBALE */
LClasseP lclasse = NIL(LClasse);
ObjetP lobjet = NIL(Objet); 

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


/*-------------------------MAKEPERSO-------------------------*/


VarDeclP makeVarDecl(char *nom, TreeP type, TreeP exprOpt)
{
    VarDeclP newVar = NEW(1, VarDecl);

    newVar->nom = nom;
    newVar->exprOpt = exprOpt;
    newVar->next = NIL(VarDecl);

    if(type != NIL(Tree))
    {
        ClasseP classeType = getClassePointer(type->u.str);
        if(classeType != NIL(Classe))
        {
            newVar->type = classeType;
        }
        else
        {
            classeType = makeClasse(type->u.str);
            newVar->type = classeType;
        }
    }
    else
    {
        fprintf(stderr, "Probleme de generation d'arbre\n");
        ClasseP classeType = makeClasse("NIL");
        newVar->type = classeType;
    }

    return newVar;
}


ClasseP makeClasse(char *nom)
{
  ClasseP classe = NEW(1, Classe);
  
  classe->nom = nom;

  classe->lparametres = NEW(1, VarDecl);
  classe->lparametres->nom = "NIL";
  classe->lparametres->type = NIL(Classe);
  classe->lparametres->exprOpt = NIL(Tree);
  classe->lparametres->next = NIL(VarDecl);

  classe->superClasse = NIL(Classe);

  classe->constructeur = NIL(Methode);

  classe->lchamps = NEW(1, VarDecl);
  classe->lchamps->nom = "NIL";
  classe->lchamps->type = NIL(Classe);
  classe->lchamps->exprOpt = NIL(Tree);
  classe->lchamps->next = NIL(VarDecl);
 
  classe->lmethodes = NEW(1, LMethode);
  classe->lmethodes->methode = NIL(Methode);
  classe->lmethodes->next = NIL(LMethode);

  return classe;
}


ObjetP makeObjet(char *nom)
{
  ObjetP objet = NEW(1, Objet);
  
  objet->nom = nom;

  objet->lchamps = NEW(1, VarDecl);
  objet->lchamps->nom = "NIL";
  objet->lchamps->type = NIL(Classe);
  objet->lchamps->exprOpt = NIL(Tree);
  objet->lchamps->next = NIL(VarDecl);
 
  objet->lmethodes = NEW(1, LMethode);
  objet->lmethodes->methode = NIL(Methode);
  objet->lmethodes->next = NIL(LMethode);

  objet->next = NIL(Objet);

  return objet;
}


MethodeP makeMethode(char *override, char *nom, VarDeclP lparametres, char *type, TreeP bloc)
{
    MethodeP newMethode = NEW(1, Methode);

    newMethode->override = (strcmp(override, "TRUE") == 0) ? TRUE : FALSE;
    newMethode->nom = nom;
    newMethode->lparametres = lparametres;

    ClasseP classeType = getClassePointer(type);
    if(classeType != NIL(Classe))
    {
        newMethode->typeDeRetour = classeType;
    }
    else
    {
        classeType = makeClasse(type);
        newMethode->typeDeRetour = classeType;
    }

    newMethode->bloc = bloc;

    return newMethode;
}


/*-------------METHODES DE STRUCTURES--------------*/


ClasseP getClassePointer(char *nom)
{
  LClasseP cur = lclasse;
  while (cur != NIL(LClasse))
  {
    if (strcmp(cur->classe->nom, nom) == 0)
    {
      return cur->classe;
    }

    cur = cur->next;
  }
  return NIL(Classe);
}


ObjetP getObjetPointer(char *nom)
{
  ObjetP cur = lobjet;
  while (cur != NIL(Objet))
  {
    if (strcmp(cur->nom, nom) == 0)
    {
      return cur;
    }

    cur = cur->next;
  }

  return NIL(Objet);
}


void addClasse(ClasseP classe)
{
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
        }
    }
}

void addObjet(ObjetP objet)
{
    if(lobjet == NIL(Objet))
    {
        lobjet = objet;      
    }
    else
    {
        ObjetP current = lobjet;
        while (TRUE) { 
            if(current->next == NULL)
            {
                current->next = objet;
                break; 
            }
            current = current->next;
        }
    }
}

void addVarDecl(VarDeclP var, VarDeclP liste)
{
    if(liste != NIL(VarDecl))
    {
        VarDeclP tmp = liste;

        while(tmp->next != NIL(VarDecl))
        {
            tmp = tmp->next;
        }

        tmp->next = var;
    }
    else
    {
      liste = var;
    }
}


LMethodeP addMethode(MethodeP methode, LMethodeP liste)
{
    LMethodeP newListe = NEW(1, LMethode);
    newListe->methode = methode;
    newListe->next = liste;

    return newListe;
}


void stockerClasse(TreeP arbreLClasse, bool verbose)
{
    TreeP courant = arbreLClasse;
    makeClassesPrimitives();

    while(courant != NIL(Tree))
    {
        TreeP arbreClasse = getChild(courant, 0);
        if(arbreClasse->op == YCLASS)
        {
            ClasseP tmp = makeClasse(getChild(arbreClasse, 0)->u.str); 
            addClasse(tmp);
            courant = getChild(courant, 1);
        }
        else
        {
            ObjetP tmp = makeObjet(getChild(arbreClasse, 0)->u.str);
            addObjet(tmp);
            courant = getChild(courant, 1);
        }
    }
    
    initClasse(arbreLClasse);

    if(verbose)
    {
        printf("----------------------------LES CLASSES----------------------------\n");
        printLClasse();
        printf("\n");
        printf("----------------------------LES OBJETS----------------------------\n");
        printObjet();
        printf("\n");
    }
}


/*Creation des types primitifs Integer, String et Void*/
void makeClassesPrimitives()
{
  ClasseP integer = makeClasse("Integer");
  ClasseP string = makeClasse("String");
  ClasseP voidC = makeClasse("Void"); 

  /* printLClasse(); */

  TreeP exprOpt = NIL(Tree);
  TreeP type = NEW(1, Tree);

  /*Initialisation d'Integer*/

  type->u.str = "Integer";
  ParamP paramInt = makeVarDecl("val", type, exprOpt);

  /*Constructeur du type Integer*/
  MethodeP constrInt = NEW(1, Methode);
  constrInt->override = FALSE;
  constrInt->nom = "Integer";
  constrInt->lparametres = NIL(VarDecl);
  constrInt->typeDeRetour = integer;
  constrInt->bloc = NIL(Tree);

  /*methode toString*/
  MethodeP toString = NEW(1, Methode);
  toString->override = FALSE;
  toString->nom = "toString";
  constrInt->lparametres = NIL(VarDecl);
  toString->typeDeRetour = string;
  toString->bloc = NIL(Tree);


  integer->lparametres = paramInt;
  integer->superClasse = NIL(Classe);
  integer->constructeur = constrInt;
  integer->lchamps = paramInt;
  integer->lmethodes->methode = toString;
  integer->lmethodes->next = NIL(LMethode);
  


  /*Initialisation de String*/

  type->u.str = "String";
  ParamP paramStr = makeVarDecl("str", type, exprOpt);

  /*Constructeur du type String*/
  MethodeP constrStr = NEW(1, Methode);
  constrStr->override = FALSE;
  constrStr->nom = "String";
  constrStr->lparametres = NIL(VarDecl);
  constrStr->typeDeRetour = string;
  constrStr->bloc = NIL(Tree);
  

  string->lparametres = paramStr;
  string->superClasse = NIL(Classe);
  string->constructeur = constrStr;
  string->lchamps = paramStr;
  string->lmethodes = NIL(LMethode);


  /* TODO
  * methode print
  * methode println
  */

  addClasse(integer);
  addClasse(string);
  addClasse(voidC);
}


void initClasse(TreeP arbreLClasse)
{
    ClasseP bufferClasse = NIL(Classe);
    ObjetP bufferObj = NIL(Objet);
    TreeP arbreCourant = arbreLClasse;

    while(arbreCourant != NIL(Tree))
    {
        TreeP arbreClasse = getChild(arbreCourant, 0);
        if(arbreClasse->op == YCLASS)
        { 
            bufferClasse = getClassePointer(getChild(arbreClasse, 0)->u.str); 

            TreeP arbreExtendOpt = getChild(getChild(arbreCourant, 0), 2);
            if(arbreExtendOpt != NIL(Tree))
                bufferClasse->superClasse = getClassePointer(getChild(arbreExtendOpt, 0)->u.str); 
        

            TreeP arbreLParam = getChild(arbreClasse, 1);

            if(arbreLParam != NIL(Tree))
            {
                ParamP lparam = NEW(1, VarDecl);

                while(arbreLParam->op == YLPARAM)
                {
                    ParamP tmp = getChild(arbreLParam, 0)->u.lvar;
                    addVarDecl(tmp, lparam);

                    arbreLParam = getChild(arbreLParam, 1);
                }
            
                ParamP tmp = arbreLParam->u.lvar;
                addVarDecl(tmp, lparam);

                bufferClasse->lparametres = lparam;
            }

            
            TreeP arbreBloc = getChild(arbreClasse, 4); 

            ChampP lchamps = NIL(VarDecl);
            LMethodeP lmethodes = NIL(LMethode);

            if(arbreBloc != NIL(Tree))
            {
                lchamps = NEW(1, VarDecl);
                /* lmethodes = NEW(1, LMethode); */ 

                TreeP arbreChampMethode = arbreBloc;

                while(arbreChampMethode->op == LDECLMETH)
                {
                    TreeP declChampMethode = getChild(arbreChampMethode, 1);

                    if(declChampMethode->op == YDECLC)
                    {
                        VarDeclP tmp = declChampMethode->u.lvar;
                        addVarDecl(tmp, lchamps);
                    }
                    else
                    {
                        TreeP arbreParamMeth = getChild(declChampMethode, 2);
                        ParamP lparam = NIL(VarDecl);

                        if(arbreParamMeth != NIL(Tree))
                        {
                            lparam = NEW(1, VarDecl);

                            while(arbreParamMeth->op == YLPARAM)
                            {
                                ParamP tmp = getChild(arbreParamMeth, 0)->u.lvar;
                                addVarDecl(tmp, lparam);

                                arbreParamMeth = getChild(arbreParamMeth, 1);
                            }
            
                            ParamP tmp = arbreParamMeth->u.lvar;
                            addVarDecl(tmp, lparam);
                        }

                        char *typeDeRetour = "Void";
                        if(declChampMethode->op == DMETHODEL)
                        {
                            typeDeRetour = getChild(declChampMethode, 3)->u.str;
                        }
                        else
                        {
                            TreeP typeOpt = getChild(declChampMethode, 3);
                            if(typeOpt != NIL(Tree))
                                typeDeRetour = typeOpt->u.str;
                        }


                        MethodeP tmp = makeMethode(getChild(declChampMethode, 0)->u.str, getChild(declChampMethode, 1)->u.str, lparam, typeDeRetour, getChild(declChampMethode, 4));
                        lmethodes = addMethode(tmp, lmethodes);
                    }

                    arbreChampMethode = getChild(arbreChampMethode, 0);
                }


                if(arbreChampMethode->op == YDECLC)
                {
                    VarDeclP tmp = arbreChampMethode->u.lvar;
                    addVarDecl(tmp, lchamps);
                }
                else
                {
                    TreeP arbreParamMeth = getChild(arbreChampMethode, 2);
                    ParamP lparam = NIL(VarDecl);

                    if(arbreParamMeth != NIL(Tree))
                    {
                        lparam = NEW(1, VarDecl);

                        while(arbreParamMeth->op == YLPARAM)
                        {
                            ParamP tmp = getChild(arbreParamMeth, 0)->u.lvar;
                            addVarDecl(tmp, lparam);

                            arbreParamMeth = getChild(arbreParamMeth, 1);
                        }
            
                        ParamP tmp = arbreParamMeth->u.lvar;
                        addVarDecl(tmp, lparam);
                    }

                    char *typeDeRetour = "Void";
                    if(arbreChampMethode->op == DMETHODEL)
                    {
                          typeDeRetour = getChild(arbreChampMethode, 3)->u.str;
                    }
                    else
                    {
                        TreeP typeOpt = getChild(arbreChampMethode, 3);
                        if(typeOpt != NIL(Tree))
                            typeDeRetour = typeOpt->u.str;
                    }

                    MethodeP tmp = makeMethode(getChild(arbreChampMethode, 0)->u.str, getChild(arbreChampMethode, 1)->u.str, lparam, typeDeRetour, getChild(arbreChampMethode, 4));
                    lmethodes = addMethode(tmp, lmethodes);
                }
            }

            /* TODO 
            * bufferClasse->constructeur
            */ 

            bufferClasse->lchamps = lchamps;
            bufferClasse->lmethodes = lmethodes;

            arbreCourant = getChild(arbreCourant, 1);
        }
        else
        {
            bufferObj = getObjetPointer(getChild(arbreClasse, 0)->u.str);

            TreeP arbreBloc = getChild(arbreClasse, 1); 

            ChampP lchamps = NIL(VarDecl);
            LMethodeP lmethodes = NIL(LMethode);

            if(arbreBloc != NIL(Tree))
            {
                lchamps = NEW(1, VarDecl);

                TreeP arbreChampMethode = arbreBloc;

                while(arbreChampMethode->op == LDECLMETH)
                {
                    TreeP declChampMethode = getChild(arbreChampMethode, 1);

                    if(declChampMethode->op == YDECLC)
                    {
                        VarDeclP tmp = declChampMethode->u.lvar;
                        addVarDecl(tmp, lchamps);

                        arbreChampMethode = getChild(arbreChampMethode, 0);
                    }
                    else
                    {
                        TreeP arbreParamMeth = getChild(declChampMethode, 2);
                        ParamP lparam = NIL(VarDecl);

                        if(arbreParamMeth != NIL(Tree))
                        {
                            lparam = NEW(1, VarDecl);

                            while(arbreParamMeth->op == YLPARAM)
                            {
                                ParamP tmp = getChild(arbreParamMeth, 0)->u.lvar;
                                addVarDecl(tmp, lparam);

                                arbreParamMeth = getChild(arbreParamMeth, 1);
                            }
            
                            ParamP tmp = getChild(arbreParamMeth, 0)->u.lvar;
                            addVarDecl(tmp, lparam);
                        }


                        char *typeDeRetour = "Void";
                        if(declChampMethode->op == DMETHODEL)
                        {
                            typeDeRetour = getChild(declChampMethode, 3)->u.str;
                        }
                        else
                        {
                            TreeP typeOpt = getChild(declChampMethode, 3);
                            if(typeOpt != NIL(Tree))
                                typeDeRetour = typeOpt->u.str;
                        }

                        MethodeP tmp = makeMethode(getChild(declChampMethode, 0)->u.str, getChild(declChampMethode, 1)->u.str, lparam, typeDeRetour, getChild(declChampMethode, 4));
                        lmethodes = addMethode(tmp, lmethodes);

                        arbreChampMethode = getChild(arbreChampMethode, 0);
                    }
                }

                if(arbreChampMethode->op == YDECLC)
                {
                    VarDeclP tmp = arbreChampMethode->u.lvar;
                    addVarDecl(tmp, lchamps);
                }
                else
                {
                    TreeP arbreParamMeth = getChild(arbreChampMethode, 2);
                    ParamP lparam = NIL(VarDecl);

                    if(arbreParamMeth != NIL(Tree))
                    {
                        lparam = NEW(1, VarDecl);

                        while(arbreParamMeth->op == YLPARAM)
                        {
                            ParamP tmp = getChild(arbreParamMeth, 0)->u.lvar;
                            addVarDecl(tmp, lparam);

                            arbreParamMeth = getChild(arbreParamMeth, 1);
                        }
            
                        ParamP tmp = getChild(arbreParamMeth, 0)->u.lvar;
                        addVarDecl(tmp, lparam);
                    }

                    char *typeDeRetour = "Void";
                    if(arbreChampMethode->op == DMETHODEL)
                    {
                          typeDeRetour = getChild(arbreChampMethode, 3)->u.str;
                    }
                    else
                    {
                        TreeP typeOpt = getChild(arbreChampMethode, 3);
                        if(typeOpt != NIL(Tree))
                            typeDeRetour = typeOpt->u.str;
                    }

                    MethodeP tmp = makeMethode(getChild(arbreChampMethode, 0)->u.str, getChild(arbreChampMethode, 1)->u.str, lparam, typeDeRetour, getChild(arbreChampMethode, 4));
                    lmethodes = addMethode(tmp, lmethodes);
                }

                bufferObj->lchamps = lchamps;
                bufferObj->lmethodes = lmethodes;
            }

            arbreCourant = getChild(arbreCourant, 1);
        }
    }
}


/*---------------------------C'EST GOOD----------------------------*/


void compile(TreeP arbreLClasse, TreeP main)
{
    if(arbreLClasse != NIL(Tree))
    {
        stockerClasse(arbreLClasse, TRUE);
    }
}










/*---------------------------AFFICHAGE----------------------------*/


void printVarDecl(VarDeclP lvar)
{
    VarDeclP tmp = lvar; 
    while(tmp != NIL(VarDecl))
    {
          if(tmp->type != NIL(Classe))
              printf("\t%s : %s\n", tmp->nom, tmp->type->nom);
          tmp = tmp->next;
    }
}


void printLClasse()
{
    LClasseP tmp = lclasse;
    while(tmp != NIL(LClasse))
    {
        printf("#####################################\n");
        printf("Classe : %s\n", tmp->classe->nom);
        if(tmp->classe->superClasse != NIL(Classe))
        {
          printf("\nSuperClasse : %s\n", tmp->classe->superClasse->nom);
        }
        else
        {
          printf("\nSuperClasse : NIL\n");
        }
        printf("\n");
        printf("Parametres :\n");
        printVarDecl(tmp->classe->lparametres); 
        printf("\n");
        printf("Champs :\n");
        printVarDecl(tmp->classe->lchamps);
        printf("\n");
        printf("Methodes :\n\n");
        printLMethode(tmp->classe->lmethodes);
        printf("\n");

        tmp = tmp->next;
    }
}


void printObjet()
{
    ObjetP tmp = lobjet;
    while(tmp != NIL(Objet))
    {
        printf("#####################################\n");
        printf("Objet : %s\n", tmp->nom);
        printf("\n");
        printf("Champs :\n");
        printVarDecl(tmp->lchamps);
        printf("\n");
        printf("Methodes :\n\n");
        printLMethode(tmp->lmethodes);
        printf("\n");

        tmp = tmp->next;
    }
}


void printMethode(MethodeP methode)
{
    if(methode != NIL(Methode))
    {
        printf("-------Methode %s-------\n", methode->nom);
        char *override = (methode->override == TRUE) ? "TRUE" : "FALSE";
        printf("Override : %s\n", override);
        printf("Parametres :\n");
        printVarDecl(methode->lparametres);
        if(methode->typeDeRetour != NIL(Classe))
            printf("Type de retour : %s\n", methode->typeDeRetour->nom);
        else
            printf("Type de retour : indefini (void ?)\n");                 /* TODO */
        printf("\n");
    }
}


void printLMethode(LMethodeP lmethode)
{
    LMethodeP tmp = lmethode;
    while(tmp != NIL(LMethode))
    {
          MethodeP methode = tmp->methode;
          printMethode(methode);
          printf("\n");
          tmp = tmp->next;
    }
}

void afficherProgramme(TreeP tree) 
{
    if(tree != NIL(Tree)) 
    {
        switch (tree->op) 
        {

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
          printVarDecl(tree->u.lvar);
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

        case DMETHODEL :
          printf("\nDMETHODEL def ");
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
          printVarDecl(tree->u.lvar);
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

        case OVERRIDE :
          printf(" override ");
          break;

        default :
          printf("Bonjour Monsieur Voisin, si ce message s'affiche, c'est qu'il existe un cas inconnu dans la construction de l'arbre. Veuillez nous en excuser. Cordialement.");
          break;
        }
    }
}
