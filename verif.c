#include <string.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char*);
extern void setError(int code);

extern LClasseP lclasse;
extern ScopeP env;
extern int nbErreur;

/* extern int yylineno; */


/* verifie la declaration d'une classe */
bool checkClassDefine(char* nom)
{
    if(nom != NULL)
    {
        LClasseP tmp = lclasse;

        while(tmp != NIL(LClasse))
        {
            if(tmp->classe != NIL(Classe))
            {
                if(strcmp(tmp->classe->nom, nom) == 0)
                {
                    return TRUE;
                }
            }
            
            tmp = tmp->next;
        }
    }
   
    fprintf(stderr, "Erreur checkClassDefine :\n");
    fprintf(stderr, "\t> la classe %s est indefini\n\n", nom);
    nbErreur++;
    return FALSE;
}


/* Verifie la portee d'une variable */
bool checkPortee(LVarDeclP lvar, char* nom)
{
    if(nom != NULL)
    {
        while(lvar != NIL(LVarDecl))
        {
            if(strcmp(lvar->var->nom, nom) == 0)
            {
                return TRUE;
            }
            lvar= lvar->next;
        }

        fprintf(stderr, "Erreur checkPortee :\n");
        fprintf(stderr, "\t> la variable %s est indefini\n\n", nom);
        nbErreur++;

        return FALSE;
    }

    fprintf(stderr, "Erreur checkPortee :\n");
    fprintf(stderr, "\t> la variable est indefini (nom = NIL)\n\n");
    nbErreur++;

    return FALSE;
}


/* verification contextuelle d'un bloc */
bool checkBlocMain(TreeP bloc, ClasseP classe)
{
    int* i = NEW(1, int);
    *i = 0;

    bool check = TRUE;

    if(bloc != NIL(Tree))
    {
        LVarDeclP tmp = NIL(LVarDecl);

        switch(bloc->op)
        {
            case YCONT :
              /* ajoute dans l'environnement la liste de declaration, si une declaration est fausse addEnv renvoie false */
              tmp = makeLParam(getChild(bloc, 0), i);

              if(tmp != NIL(LVarDecl))
                check = addEnv(tmp, classe) && check;

              check = checkBlocMain(getChild(bloc, 1), classe) && check;

              /* met a jour l'env */
              removeEnv(*i);
              break;
            
            case LINSTR :
              /* verification d'une liste d'instructions */
              check = checkBlocMain(getChild(bloc, 0), classe) && check;
              check = checkBlocMain(getChild(bloc, 1), classe) && check;
              break;

            case YITE :
              /* verification d'un if then else */
              check = checkExpr(getChild(bloc, 0), classe) && check;  
              check = checkBlocMain(getChild(bloc, 1), classe) && check;
              check = checkBlocMain(getChild(bloc, 2), classe) && check;
              break;

            case EAFF :
              /* verification d'une affectation */
              check = checkBlocMain(getChild(bloc, 0), classe) && check;
              check = checkExpr(getChild(bloc, 1), classe) && check;
              break;

            case SELEXPR :
              /* verification d'une selection */
              check = checkSelection(bloc) && check;
              break;

            case Id :
              /* verification de la portee */
              check = checkPortee(env->env, bloc->u.str) && check;  
              break;  

            case YEXPR :
              /* verification d'une expression */
              check = checkExpr(getChild(bloc, 0), classe) && check;  
              break;

            case YRETURN :
              break;

            default :
              fprintf(stderr, "Erreur etiquette dans checkBlocMain :\n");
              fprintf(stderr, "\t> etiquette %d inconnu\n\n", bloc->op);
              check = FALSE;
              break;
        }
    }

    free(i);
    return check;
}


/* verifie la validite d'une expression */
/* TODO : ne fait que la portee, faut faire le reste */
bool checkExpr(TreeP tree, ClasseP classe)
{
    bool check = TRUE;

    if(tree != NIL(Tree))
    {
        char* nom;

        ClasseP typeG;
        ClasseP typeD;

        switch (tree->op)
        {
            case Id :
                /* verification de la portee d'une id */
                nom = tree->u.str;
                if(strcmp(nom, "this") != 0 && strcmp(nom, "super") != 0 && strcmp(nom, "result") != 0)
                    check = checkPortee(env->env, nom) && check;

                /* TODO : super et result */
                break;

            case Classname :
                /* verification d'un nom de classe ou d'objet */
                nom = tree->u.str;
                check = checkClassDefine(nom) && check;
                break;

            case Chaine :
            case Cste :
                break;
        
            case ADD :
                /* TODO : verif avec le truc unaire */
            case SUB :
            case MUL :
            case DIV :    
            case EQ :
            case NE :
            case SUP :
            case SUPE :
            case INF :
            case INFE :
                /* verification d'une expression arithmetique */
                /* verifie que les deux expressions sont correctes */
                check = checkExpr(getChild(tree, 0), classe) && check;
                check = checkExpr(getChild(tree, 1), classe) && check;

                /* verifie que les types des deux expressions sont des Integer */
                typeG = getType(getChild(tree, 0), classe);
                typeD = getType(getChild(tree, 1), classe);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if(!(strcmp(typeG->nom, "Integer") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [op arithmetique] :\n");
                        fprintf(stderr, "\t> le fils gauche %s n'est pas un Integer\n\n", typeG->nom);
                        nbErreur++;
                        /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                        check = FALSE;             
                    }

                    if((!strcmp(typeD->nom, "Integer") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [op arithmetique] :\n");
                        fprintf(stderr, "\t> le fils droit %s n'est pas un Integer\n\n", typeD->nom);
                        nbErreur++;
                        /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                        check = FALSE;             
                    }
                }
                else
                {
                    fprintf(stderr, "Erreur verification d'une expression [op arithmetique] :\n");
                    fprintf(stderr, "\t> type inconnu\n\n");
                    nbErreur++;
                    /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                    check = FALSE;
                }
                break;

            case USUB :
                /* verifie l'expression suivant le moins unaire */
                check = checkExpr(getChild(tree, 1), classe) && check; 

                /* verifie le type de l'expression */
                typeD = getType(getChild(tree, 1), classe);
                if(typeD != NIL(Classe))
                {
                    if((!strcmp(typeD->nom, "Integer") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [- unaire] :\n");
                        fprintf(stderr, "\t> le fils droit %s n'est pas un Integer\n\n", typeD->nom);
                        nbErreur++;
                        /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                        check = FALSE;             
                    }
                }
                else
                {
                    fprintf(stderr, "Erreur verification d'une expression [- unaire] :\n");
                    fprintf(stderr, "\t> type inconnu\n\n");
                    nbErreur++;
                    /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                    check = FALSE;
                }
                break;

            case CONCAT :
                /* verification d'une concatenation */
                /* verifie que les deux expressions sont correctes */
                check = checkExpr(getChild(tree, 0), classe) && check;
                check = checkExpr(getChild(tree, 1), classe) && check;

                /* verifie que les types des expressions sont des String */
                typeG = getType(getChild(tree, 0), classe);
                typeD = getType(getChild(tree, 1), classe);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if(!(strcmp(typeG->nom, "String") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [concatenation] :\n");
                        fprintf(stderr, "\t> le fils gauche %s n'est pas un String\n\n", typeG->nom);
                        nbErreur++;
                        /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                        check = FALSE;             
                    }

                    if((!strcmp(typeD->nom, "String") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [concatenation] :\n");
                        fprintf(stderr, "\t> le fils droit %s n'est pas un String\n\n", typeD->nom);
                        nbErreur++;
                        /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                        check = FALSE;             
                    }
                }
                else
                {
                    fprintf(stderr, "Erreur verification d'une expression [op arithmetique] :\n");
                    fprintf(stderr, "\t> type inconnu\n\n");
                    nbErreur++;
                    /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                    check = FALSE;
                }
                break;

            case ECAST :
                /* verification d'un cast */
                check = checkExpr(getChild(tree, 0), classe) && check;
                check = checkExpr(getChild(tree, 1), classe) && check;
                check = checkCast(getClassePointer(getChild(tree, 1)->u.str), getChild(tree, 0)->u.str, classe) && check; 
                break;

            case EINST :
                /* verification d'une allocation */
                check = checkExpr(getChild(tree, 0), classe) && check;

                /* TODO : verifier le nombre/type d'argument du constructeur */
                check = checkExpr(getChild(tree, 1), classe) && check;
                break;

            case SELEXPR :      
                /* verification d'une selection */
                check = checkSelection(tree) && check;
                break;

            case EENVOI :
                /* verification d'un envoi */
                check = checkExpr(getChild(tree, 0), classe) && check;
                check = checkMethodes(getType(getChild(tree, 0), classe), getChild(getChild(tree, 1), 0)->u.str,getChild(getChild(tree, 1), 1) ) && check;
                break;

            case YLEXPR :
                /* verification d'une liste d'expressions */ 
                check = checkExpr(getChild(tree, 0), classe) && check;
                check = checkExpr(getChild(tree, 1), classe) && check;
                break; 

            default :
                fprintf(stderr, "Erreur etiquette dans checkExpr :\n");
                fprintf(stderr, "\t> etiquette %d inconnu\n\n", tree->op);
                check = FALSE;
                break;
        }
    }

    return check;
}


/* verifie la validite d'une selection */
bool checkSelection(TreeP selection)
{
    /* TODO */

    /* soit classe.attribut dans une methode de other, si other n'herite pas de classe, alors 
    classe.attribut est faux car l'attribut n'est pas visible dans other */

    return TRUE;
}



/* retourne le nom du type d'une expression correcte ou NIL */
ClasseP getType(TreeP expr, ClasseP classe)
{
    ClasseP type = NIL(Classe);

    if(expr != NIL(Tree))
    {
        ClasseP typeG;
        ClasseP typeD;

        char *nom;
        LVarDeclP tmp = NIL(LVarDecl);

        switch (expr->op)
        {
            case Id :
                /* retourne le type de l'id : si l'id correspond a this, retourne la classe actuel */
                if(strcmp(expr->u.str, "this") == 0)
                {
                    if(classe != NIL(Classe))
                        type = classe;
                    else
                    {
                        fprintf(stderr, "Erreur de type :\n");
                        fprintf(stderr, "\t> this est indefini\n\n");
                    }
                }
                else
                {
                    type = getTypeId(expr->u.str);
                }
                break;

            case Classname :
                type = getClassePointer(expr->u.str);
                break;

            case Chaine :
                type = getClassePointer("String");
                break;

            case Cste :
                type = getClassePointer("Integer");
                break;
        
            case ADD :
                /* TODO : verif avec le truc unaire */
            case SUB :
            case MUL :
            case DIV :            
            case EQ :
            case NE :
            case SUP :
            case SUPE :
            case INF :
            case INFE :
                /* retourne Integer si l'operation arithmetique est entre deux Integer */
                typeG = getType(getChild(expr, 0), classe);
                typeD = getType(getChild(expr, 1), classe);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if((strcmp(typeG->nom, typeD->nom) == 0) && (strcmp(typeG->nom, "Integer") == 0))
                    {
                        type = typeG;           
                    }
                    else
                    {
                        type = NIL(Classe);
                    }
                }
                else
                {
                    type = NIL(Classe);
                }
                break;

            case USUB :
                /* retourne Integer si l'operation unaire est correcte */
                typeD = getType(getChild(expr, 1), classe); 
                if(strcmp(typeD->nom, "Integer") == 0)
                    type = typeD;
                else
                    type = NIL(Classe);
                break;

            case CONCAT : 
                /* retourne String si la concatenation est entre deux String */
                typeG = getType(getChild(expr, 0), classe);
                typeD = getType(getChild(expr, 1), classe);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if((strcmp(typeG->nom, typeD->nom) == 0) && (strcmp(typeG->nom, "String") == 0))
                    {
                        type = typeG;           
                    }
                    else
                    {
                        type = NIL(Classe);
                    }
                }
                else
                {
                    type = NIL(Classe);
                }
                break;

            case ECAST :
                /* retourne le type du cast si le cast est entre deux types existants */
                /* TODO : verifier le cast ici pour le prob de l'ex1 ? */
                typeG = getType(getChild(expr, 0), classe);
                typeD = getType(getChild(expr, 1), classe);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if((strcmp(typeG->nom, typeD->nom) == 0))
                    {
                        type = typeG;           
                    }
                    else
                    {
                        type = NIL(Classe);
                    }
                }
                else
                {
                    type = NIL(Classe);
                }
                break;

            case EINST :
                /* retourne le type de l'allocation */
                /* TODO : verifie l'allocation ici ? */
                type = getType(getChild(expr, 0), classe);
                break;

            case SELEXPR :      
                /* retourne le type de la selection ou de l'id */
                if(expr->nbChildren == 2) /* cas expr '.' id */
                {
                    /* on recupere la classe de l'expr */
                    typeG = getType(getChild(expr, 0), classe);
                    if(typeG != NIL(Classe))
                    {
                        /* on recupere le nom de l'attribut */
                        nom = getChild(expr, 1)->u.str;

                        /* on cherche l'attribut dans la classe et on retourne son type */ 
                        tmp = typeG->lchamps;
                        if(tmp != NIL(LVarDecl))
                        {
                            while(tmp != NIL(LVarDecl))
                            {
                                if(strcmp(tmp->var->nom, nom) == 0)
                                {
                                    type = tmp->var->type;
                                    break;
                                }

                                tmp = tmp->next;
                            }
                        }
                        else
                        {
                            fprintf(stderr, "Erreur de selection dans getType :\n");
                            fprintf(stderr, "\t> la liste de champs de la classe %s est NIL\n\n", typeG->nom);
                            type = NIL(Classe);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Erreur de selection dans getType :\n");
                        fprintf(stderr, "\t> la classe est indifini\n\n");
                        type = NIL(Classe);
                    }
                }
                else /* cas id */
                {
                    type = getType(getChild(expr, 0), classe);
                }
                break;

            case EENVOI :
                /* retourne le type de la methode */
                typeG = getType(getChild(expr, 0), classe);
                
                if(typeG != NIL(Classe))
                {
                    MethodeP methode = getMethodePointer(typeG, getChild(getChild(expr, 1), 0)->u.str);
                    if(methode != NIL(Methode))
                    {
                        type = methode->typeDeRetour;
                    }
                    else
                    {
                        type = NIL(Classe);
                    }
                }
                else
                {
                    type = NIL(Classe);
                }
                break;

            default :
                fprintf(stderr, "Erreur etiquette dans getType :\n");
                fprintf(stderr, "\t> etiquette %d inconnu\n\n", expr->op);
                type = NIL(Classe);
                break;
        }
    }

    return type;
}


/* retourne le type d'un id */
ClasseP getTypeId(char* nom)
{
    if(env != NIL(Scope))
    {
        LVarDeclP tmp = env->env;
        while(tmp != NIL(LVarDecl))
        {
            if(strcmp(nom, tmp->var->nom) == 0)
            {
                return tmp->var->type;
            }
            tmp = tmp->next;
        }
    }
    return NIL(Classe);
}  


/* verification du type des variables et des affectation associees */
bool setEnvironnementType(LVarDeclP var, ClasseP classe)               
{
    bool check = TRUE;
    LVarDeclP tmp = var;
    while(tmp != NIL(LVarDecl))
    {
        if(tmp->var->type != NIL(Classe))
        {
            /* recupere le pointeur vers le type de la variable dans la liste lclasse */
            ClasseP type = getClassePointer(tmp->var->type->nom);
            if(type != NIL(Classe))
            {
                tmp->var->type = type;

                /* verifie l'affectation opt */
                if(tmp->var->exprOpt != NIL(Tree))
                {
                    check = checkAff(tmp->var, tmp->var->exprOpt, classe);
                }
            }
            else
            {
                fprintf(stderr, "Erreur de verification de type :\n");
                fprintf(stderr, "\t> le type %s est inconnu\n\n", tmp->var->type->nom);
                nbErreur++;
                check = FALSE;
            }
        }
        else /* 99% sur que ce cas est impossible */
        {
            fprintf(stderr, "Erreur de verification de type :\n");
            fprintf(stderr, "\t> le type de %s = NIL\n\n", tmp->var->nom);
            nbErreur++;
            check = FALSE;
        }
        
        tmp = tmp->next;
    }

    return check;
}


/* verification du bloc contenant les classes et les objets */
bool checkBlocClasse(TreeP tree, ClasseP classe)
{
    int* i = NEW(1, int);
    *i = 0;

    bool check = TRUE;

    if(tree != NIL(Tree))
    {
        LVarDeclP tmp = NIL(LVarDecl);
        ClasseP superClasse = NIL(Classe);

        switch(tree->op)
        {
            case YLCLASS :
                /* parcours de la liste de classe ou d'objet */
                check = checkBlocClasse(getChild(tree, 0), classe) && check;
                check = checkBlocClasse(getChild(tree, 1), classe) && check;
                break;

            case YCLASS :
                /* verifie qu'une classe est correcte */
                classe = getClassePointer(getChild(tree, 0)->u.str);

                /* verifie que l'heritage est correct */
                TreeP extends = getChild(tree, 2);
                if(extends != NIL(Tree))
                {
                    superClasse = getClassePointer(getChild(extends, 0)->u.str);
                    if(superClasse != NIL(Classe) && superClasse->constructeur != NIL(Methode))
                    {
                        tmp = NIL(LVarDecl);
                        /* ajoute dans l'env celui de la super classe */
                        tmp = envHerite(superClasse);
                        check = addEnv(tmp, classe) && check;
                    }
                    else
                    {
                        fprintf(stderr, "Erreur d'heritage dans checkBlocClasse :\n");
                        fprintf(stderr, "\t> la classe %s est inconnu\n\n", getChild(extends, 0)->u.str);
                        nbErreur++;
                    }
                }

                /* ajoute les parametres de la classe dans l'env pour faire la verification du constructeur */
                /* TODO : verifier le nom des variables, pour ne pas avoir deux fois la meme variable dans le meme bloc */
                tmp = classe->lparametres;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe) && check;
           
                /* verification du constructeur de la classe */ 
                TreeP constructeur = getChild(tree, 3);
                check = checkBlocMain(constructeur, classe) && check;

                int tailleLParam = getTailleListeVarDecl(tmp);
                removeEnv(tailleLParam);

                /* ajoute les champs de la classe dans l'env */
                /* TODO : verifier le nom des variables, pour ne pas avoir deux fois la meme variable dans le meme bloc */
                tmp = classe->lchamps;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe) && check;

                /* verifie le blocObj de la classe */
                check = checkBlocClasse(getChild(tree, 4), classe) && check; 
                
                /* reset l'env */
                removeEnv(env->taille);
                break;

            case YOBJ :
                /* verifie qu'un objet est correct */
                classe = getClassePointer(getChild(tree, 0)->u.str);

                /* ajoute les champs de l'objet dans l'env */
                /* TODO : verifier le nom des variables, pour ne pas avoir deux fois la meme variable dans le meme bloc */
                tmp = classe->lchamps;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe) && check;

                /* verifie le blocObj de l'objet */
                check = checkBlocClasse(getChild(tree, 1), classe) && check; 

                /* reset l'env */
                removeEnv(env->taille);
                break;

            case LDECLMETH :
                /* verification d'une liste de declarations */
                check = checkBlocClasse(getChild(tree, 0), classe) && check;
                check = checkBlocClasse(getChild(tree, 1), classe) && check;
                break; 

            case YDECLC :
                /* les champs d'une classe ou d'un objet sont deja ajoutes dans l'env */
                break;

            case DMETHODEL :
                /* verification d'une methode : OverrideOpt DEF Id '(' LParamOpt ')' ':' Classname AFF Expr*/
                /* TODO : mettre dans cette partie la verification d'une declaration d'une methode 
                * reste encore a mettre : override correct (attention entre classe et objet), nom correct (pas de surcharge) 
                */

                /* ajoute dans l'env les parametres de la methode */
                tmp = makeLParam(getChild(tree, 2), i);
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe) && check;

                /* verifie le type de retour de la methode */
                check = checkClassDefine(getChild(tree, 3)->u.str) && check;

                /* verifie l'expression associee a la methode */
                check = checkExpr(getChild(tree, 4), classe) && check;

                /* met a jour l'env */
                removeEnv(*i);
                break;

            case DMETHODE :
                /* verification d'une methode : OverrideOpt DEF Id '(' LParamOpt ')' TypeCOpt IS Bloc */
                /* TODO : mettre dans cette partie la verification d'une declaration d'une methode 
                * reste encore a mettre : override correct (attention entre classe et objet), nom correct (pas de surcharge) 
                */

                /* ajoute dans l'env les parametres de la methode */
                tmp = makeLParam(getChild(tree, 2), i);
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe) && check;

                /* verifie le type de retour de la methode */
                TreeP type = getChild(tree, 3);
                if(type != NIL(Tree))
                    check = checkClassDefine(type->u.str) && check;

                /* verifie le bloc associe a la methode */
                check = checkBlocMain(getChild(tree, 4), classe) && check;

                /* met a jour l'env */
                removeEnv(*i);
                break;

            default :
                fprintf(stderr, "Erreur etiquette dans checkBlocClasse :\n");
                fprintf(stderr, "\t> etiquette %d inconnu\n\n", tree->op);
                check = FALSE;
                break;
        }
    }

    free(i);
    return check;
}


/* renvoie l'environnement de la super classe */
LVarDeclP envHerite(ClasseP classeMere)
{
    /* TODO : verifier pour un .txt ou les classes d'heritages sont dans le desordre (A2 declare avant A) */
    LVarDeclP env = NIL(LVarDecl);
    env = addVarDecl(classeMere->lchamps, env);
    return env;
}


/* ajoute dans env une liste de variable var */
bool addEnv(LVarDeclP var, ClasseP classe)
{
    bool check = TRUE;
    
    if(var != NIL(LVarDecl))
    {
        LVarDeclP tmp = var;
        while(tmp != NIL(LVarDecl))
        {
            check = addVarEnv(tmp->var, classe) & check;
            tmp = tmp->next;
        }
    }

    return check;
}


/* ajoute une variable dans env et verifie son affection opt ainsi que son type */
bool addVarEnv(VarDeclP var, ClasseP classe)
{
    bool check = TRUE;

    if(var != NIL(VarDecl))
    {
        ScopeP newEnv = NEW(1, Scope);

        LVarDeclP tmp = NEW(1, LVarDecl);
        tmp->var = var;
        tmp->next = NIL(LVarDecl);

        check = setEnvironnementType(tmp, classe);

        if(env->env == NIL(LVarDecl))
        {
            env->env = tmp;
            env->taille = env->taille + 1;     
        }
        else
        {
            newEnv->env = tmp;
            newEnv->taille = env->taille + 1;
  
            newEnv->env->next = env->env;

            env = newEnv;
        }
    }

    return check;
}


/* retire n variables de l'env */
void removeEnv(int n)
{
    if(env->taille < n)
    {
        fprintf(stderr, "Erreur removeEnv\n");
        return;
    }

    if(n > 0)
    {
        int i;
        LVarDeclP tmp = env->env;
        for(i = 0; i < n; i++)
        {
            tmp = tmp->next;
        }

        env->env = tmp;
        env->taille = env->taille - n;
    }
}


/* verifie une affectation */
bool checkAff(VarDeclP var, TreeP expr, ClasseP classe)
{
    bool check = checkExpr(expr, classe);

    if(check)
    {
        ClasseP tmp = getType(expr, classe);
        if(tmp != NIL(Classe))
        {
            return (var != NIL(VarDecl) && strcmp(var->type->nom, tmp->nom) == 0 && check);
        }

        fprintf(stderr, "Erreur verification affectation\n");
        nbErreur++;
    }
    
    /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
    return FALSE;
}


/* retourne la taille d'une liste de varDecl */
int getTailleListeVarDecl(LVarDeclP liste)
{
    int i = 0;
    if(liste != NIL(LVarDecl))
    {
        LVarDeclP tmp = liste;
        while(tmp != NIL(LVarDecl))
        {
            i++;
            tmp = tmp->next;
        }
    }

    return i;
}

















































bool checkArguments(LParamP larg, LParamP largbis)
{
    bool retour = TRUE;
    LParamP larg1 = larg;
    LParamP larg2 = largbis;
    printf("hey\n");
    while(larg1 != NIL(LParam) && larg2 != NIL(LParam))
    {
        if(larg1->var != NIL(VarDecl) && larg2->var != NIL(VarDecl) && larg1->var->type != NIL(Classe) && larg2->var->type != NIL(Classe) 
            && strcmp(larg1->var->type->nom, larg2->var->type->nom) == 0)
        {
            larg1 = larg1->next;
            larg2 = larg2->next;
        }
        else
        {
            retour = FALSE;
        }
    }
    if(larg1 != NIL(LParam) || larg2 != NIL(LParam)) retour = FALSE;
    return retour;
}

/*Renvoie true si pas de probleme d'override*/
bool checkOverrideMethode(ClasseP classe, char* nom, LParamP larg, bool isOverride)
{
    if(classe != NIL(Classe))
    {
        LMethodeP tmpMethodes = classe->lmethodes;
        while(tmpMethodes != NIL(LMethode))
        {
            if(strcmp(tmpMethodes->methode->nom,nom) == 0 )
            {
                if(checkArguments(tmpMethodes->methode->lparametres,larg))
                {
                    if(!isOverride)
                    {
                        printf("| Erreur Override | Rajouter un override dans la fonction %s de ",nom );
                        nbErreur++;
                    }
                    return TRUE;
                }
                else
                {
                    if(!isOverride)
                    {
                        printf("surcharge \n");
                        nbErreur++;
                    }
                    else
                    {
                        printf("| Erreur Override | Les paramètres de la méthode %s sont différents dans la classe %s et ",nom,classe->nom);
                        nbErreur++;                      
                    }
                    return FALSE;
                }
            }
            tmpMethodes = tmpMethodes->next;
        }
        if(classe->superClasse != NIL(Classe))
        {
            return checkOverrideMethode(classe->superClasse, nom, larg, isOverride);
        }
        else
        {
            if(isOverride)
            {
                printf("| Erreur Override | La méthode %s n'a pas été trouvée dans les Super Classes de ", nom);
                nbErreur++;
                return FALSE;
            }
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    
}

bool checkOverrideLClasse(LClasseP lclasse)
{
    bool b = TRUE;
    LClasseP tmpClasses = lclasse;
    while(tmpClasses != NIL(LClasse))
    {
            LMethodeP tmpMethodes = tmpClasses->classe->lmethodes;
            while(tmpMethodes != NIL(LMethode))
            {
                if(tmpMethodes->methode->override && !checkOverrideMethode(tmpClasses->classe->superClasse, tmpMethodes->methode->nom, tmpMethodes->methode->lparametres,tmpMethodes->methode->override))
                {
                    if(tmpClasses->classe->superClasse == NIL(Classe)) printf("| Erreur Override | La classe mère n'existe pas alors qu'il y a un override dans ");
                    printf("la classe %s \n\n", tmpClasses->classe->nom);
                    b =  FALSE;
                }
                if(!tmpMethodes->methode->override && checkOverrideMethode(tmpClasses->classe->superClasse, tmpMethodes->methode->nom, tmpMethodes->methode->lparametres,tmpMethodes->methode->override))
                {
                    printf("la classe %s \n\n", tmpClasses->classe->nom);
                    b = FALSE;
                }
                tmpMethodes = tmpMethodes->next;
            }
            tmpClasses = tmpClasses->next;
    }
    return b;
}

/*Check s'il y a un doublon dans les classes*/
bool checkDoublonClasse(LClasseP lclasse)
{
    if(lclasse != NIL(LClasse))
    {
        LClasseP tmpClasses = lclasse;
        LClasseP tmp= lclasse;
        tmpClasses = tmpClasses->next;
        while(tmpClasses != NIL(LClasse))
        {
           if(strcmp(tmpClasses->classe->nom,tmp->classe->nom)==0)     
           {
                printf("Doublon de classe concernant la classe de nom : \n\t > %s\n\n", tmp->classe->nom);
                nbErreur++;
                return FALSE;
           }
           tmpClasses = tmpClasses->next;
        }
        return checkDoublonClasse(lclasse->next);
    }
    else
    {
        return TRUE;
    }
}

/*Check s'il y a une boucle dans l'héritage des classes*/
bool checkBoucleHeritage(LClasseP lclasse)
{
    LClasseP tmpClasses = lclasse;
    while(tmpClasses != NIL(LClasse))
    {
        ClasseP tmpSuper= tmpClasses->classe->superClasse;
        while(tmpSuper != NIL(Classe))
        {
            if(strcmp(tmpClasses->classe->nom, tmpSuper->nom) == 0)
            {
                printf("Erreur boucle d'héritage :\n\t> boucle infini concernant %s\n\n", tmpClasses->classe->nom);
                nbErreur++;
                fprintf(stderr, "Pour poursuivre la compilation, résoudre cette erreur\n");
                abort();
            }
            tmpSuper = tmpSuper->superClasse;
        }
        tmpClasses = tmpClasses->next;
    }
    return TRUE;
}

/*devrait marche, à tester avec les blocs*/
bool checkCast(ClasseP classeCast, char* nom, ClasseP classe)
{
    if(classeCast != NIL(Classe))
    {
        if(strcmp(nom, "this") == 0 && classe != NIL(Classe))
        {
            nom = classe->nom;
        }

        if(strcmp(classeCast->nom, nom) == 0)
        {
            return TRUE;
        }
        else
        {
            if(classeCast->superClasse != NULL)
            {
                return checkCast(classeCast->superClasse, nom, classe);
            }
            else
            {
                fprintf(stderr, "Erreur verification d'un cast :\n");
                fprintf(stderr, "\t> %s ne peut pas etre cast en %s\n\n", classeCast->nom, nom);
                nbErreur++;
                /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
                return FALSE;
            }
        }
    }
    else
    {
        fprintf(stderr, "Erreur verification d'un cast :\n");
        fprintf(stderr, "\t> classe indefini\n\n");
        nbErreur++;

        return FALSE;
    }
}

/*Fonction utile pour un envoi : on regarde si la méthode existe dans la classe de l'objet*/
bool checkMethodes(ClasseP classe, char* nom, TreeP lparam)
{
    bool check = FALSE;

    if(classe != NIL(Classe))
    {
        LMethodeP tmp = classe->lmethodes;
        while(tmp != NIL(LMethode))
        {
            if(strcmp(tmp->methode->nom, nom) == 0)
            {
                TreeP tmplparam = lparam;
                LParamP methLParam = tmp->methode->lparametres;
                bool compatible = TRUE;
                while(tmplparam != NIL(Tree) && tmplparam->op == YLEXPR && methLParam != NIL(LParam))
                {
                    if(methLParam->var->exprOpt != NIL(Tree))
                    {
                        if(strcmp(getType(getChild(tmplparam,0),NIL(Classe))->nom, methLParam->var->type->nom)==0)
                        {
                            methLParam= methLParam->next;
                            tmplparam =  getChild(tmplparam,1);
                        }
                        else
                        {
                            methLParam= methLParam->next;
                        }
                    }
                    else
                    {
                        if(strcmp(getType(getChild(tmplparam,0),NIL(Classe))->nom, methLParam->var->type->nom)!=0)
                        {
                            compatible = FALSE;
                        }
                        methLParam= methLParam->next;
                        tmplparam =  getChild(tmplparam,1);
                    }
                    
                }
                if(methLParam != NIL(LParam) && methLParam->var->exprOpt == NIL(Tree) && getType(tmplparam, NIL(Classe))!= methLParam->var->type)
                {
                    compatible = FALSE;
                }
                
                if(compatible) check = TRUE;
                break;
            }
            tmp = tmp->next;
        }

        if(!check && classe->superClasse != NIL(Classe))
        {
            check = checkMethodes(classe->superClasse, nom, lparam);
        }
        

        if(check == FALSE)
        {
            fprintf(stderr, "Erreur verification methode :\n");
            fprintf(stderr, "\t> %s n'est pas une methode de %s\n\n", nom, classe->nom);
            nbErreur++;
            /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
        }
        /* TODO : ajouter la verif des parametres */
    }
    else
    {
        fprintf(stderr, "Erreur verification methode :\n");
        fprintf(stderr, "\t> le type de l'envoi est indefini\n\n");
        nbErreur++;
    }
        
    return check;
}



































/*METHODES UTILES POUR LES VERIFICATIONS CONTEXTUELLES : YASSINE AMRAOUI
Yassine AMRAOUI : j'ai cree ces methodes qui servent à l'analyse contextuelle
Et cf. les TODO apres les methodes qui sont des idees de verifications supplementaires
Recuperation de la classe (ClasseP) portant le nom passe en parametre*/
/**ClasseP getClasse(ClasseP lclasse, char *nom)
{
    if(nom!= NULL) {
        ClasseP buffer = lclasse;
        while(buffer!=NULL)
        {
            if(strcmp(nom, buffer->nom)==0) return buffer;
            buffer=buffer->next;
        }
        printf("\nClasse %s non existante...", nom);
        setError(CONTEXT_ERROR);
    }
    return NIL(Classe);
}*/

/*Renvoie true si pas de probleme d'override*/
/*bool checkOverrideMethode(ClasseP classe, char* nom)
{
    if(classe != NIL(Classe))
    {
        LMethodeP tmpMethodes = classe->lmethodes;
        while(tmpMethodes != NIL(LMethode))
        {
            if(strcmp(tmpMethodes->methode->nom,nom) == 0 )
            {
                return TRUE;
            }
            tmpMethodes = tmpMethodes->next;
        }
        if(classe->superClasse != NIL(Classe))
        {
            return checkOverrideMethode(classe->superClasse, nom);
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}
bool checkOverrideLClasse(LClasseP lclasse)
{ 
    LClasseP tmpClasses = lclasse;
    while(tmpClasses != NIL(LClasse))
    {
        LMethodeP tmpMethodes = lclasse->classe->lmethodes;
        while(tmpMethodes != NIL(LMethode))
        {
            if(tmpMethodes->methode ==  NIL(Methode)) printf("fzebfzeufeybfeyb\n");
            printf("override : %d de %s de la classe%s\n", tmpMethodes->methode->override, tmpMethodes->methode->nom,lclasse->classe->nom);
            if(tmpMethodes->methode->override)
            {
                if(!checkOverrideMethode(lclasse->classe->superClasse, tmpMethodes->methode->nom))
                {
                    return FALSE;
                }
                
            }
            printf("next\n");
            tmpMethodes = tmpMethodes->next;
        } 
        lclasse = lclasse->next;
    }
    return TRUE;
}*/
    /*if(lclasse != NULL) 
    {
        bool booleen = checkOverride(lclasse->next); */
        /*Appel recursif pour parcourir l'ensemble de la liste
        Maintenant, on check les override classe par classe (depuis la fin de lclasse par recursivite)
        Si la classe n'a pas de classe mere, override interdit : on va checker*/

       /* if(lclasse->classe->mereOpt == NULL) {
            MethodeP lmethodestemp = lclasse->classe->lmethodes; 
            while(lmethodestemp != NULL) {
                if(lmethodestemp->methode->override) {printf("\nErreur Override : la classe %s n'a pas de classe mere\n",lclasse->classe->nom); retour = FALSE;}
                lmethodestemp = lmethodestemp->next;
            }
        }
        
        else {
            MethodeP lmethodestemp = lclasse->classe->lmethodes;
            while(lmethodestemp != NULL) {
                if(lmethodestemp->methode->override && !checkSuper(lclasse->classe->mereOpt, lmethodestemp)) {printf("Erreur Override : la methode %s de la classe %s n'est pas presente dans la classe mere %s\n", lmethodestemp->methode, lclasse->classe->nom, lclasse->classe->mereOpt->nom); retour = FALSE;}
                lmethodestemp = lmethodestemp->next;
            }
        }
        return retour;  
    }
    return TRUE;   
}*/
/*Renvoie true si la methode passe en parametre est trouvee dans la classe mere*/
/*bool checkSuper(ClasseP lclasse, MethodeP lmethode) { 
    if(lclasse != NULL) {
        bool retour = checkSuper(lclasse->mereOpt, lmethode);  *//*Appel recursif pour parcourir l'ensemble de la liste    
        *//*if(checkMethodeInClasse(lmethode, lclasse)) return TRUE;    
        else return retour;
    }
    return FALSE;
}*/
/*Renvoie true si la méthode passée en paramètre est dans la classe*/
/*bool checkMethodeInClasse(MethodeP lmethode, ClasseP lclasse) { 
    bool retour = FALSE;
    MethodeP methodebuffer = lclasse->lmethodes->methode;
    
    while(methodebuffer != NULL) {
        if(strcmp(lmethode->methode->nom, methodebuffer->nom) == 0) {
            if(lmethode->typeDeRetour != NULL && methodebuffer->typeDeRetour != NULL && strcmp(lmethode->typeDeRetour, methodebuffer->typeDeRetour) == 0) {
                if(checkArguments(lmethode->args, methodebuffer->args)) retour = TRUE;
            }
            else if(lmethode->typeDeRetour == NULL && methodebuffer->typeDeRetour == NULL) {
                if(checkArguments(lmethode->larg, methodebuffer->larg)) retour = TRUE;
            }
        }
        methodebuffer = methodebuffer->next;
    }
    return retour;
}*/
    /*
Renvoie true si même arguments*/





/*bool checkArguments(LParamP larg1, LPAram larg2) {
    bool retour = TRUE;
    while(arg1 != NULL && arg2 != NULL) {
        if(arg1->attibut->type != NULL && arg2->attribut->type != NULL && strcmp(arg1->attribut->type, arg2->attribut->type) != 0) retour = FALSE;
        arg1 = arg1->next;
        arg2 = arg2->next;
    }
    if(arg1 != NULL || arg2 != NULL) retour = FALSE;
    return retour;
}    */
/*
TODO : methodes permettant d'update les listes : par exemple, completer l'attribut mereOpt, les types
TODO : methode permettant de verifier s'il existe plusieurs classes avec le même nom
TODO : methode permettant de voir si un Id est dans l'environnement
*/
/*Yassine AMRAOUI : ce que j'ai ameliore et ajoute
Verifier si une méthode est bien definie
*/
/*bool checkMethodes(ClasseP classe, char* nom)
{
    while(classe->lmethodes != NULL)
    {
        if(strcmp(classe->lmethodes->methode->nom,nom)==0)
        {
            return TRUE;
        }
        else
        {
            classe->lmethodes = classe->lmethodes->next;
        }
    }
    if(classe->mereOpt != NULL)
    {
        return checkMethodes(classe->mereOpt, nom);
    }
    else
    {
        return FALSE;
    }
}*/
/*Verifie l'heritage d'une classe*/
/*bool checkHeritageClass(ClasseP classe, char* nom)
{
    if(strcmp(classe->nom,nom)==0)
    {
        return TRUE; 
    }
    else
    {
        if(classe->mereOpt != NULL)
        {
            return checkHeritageClass(classe->mereOpt,nom);
        }
        else
        {
            return FALSE;
        }
    }
}
*/