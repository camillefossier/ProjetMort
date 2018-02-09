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

        fprintf(stderr, "Erreur checkClassDefine :\n");
        fprintf(stderr, "\t> la classe %s n'existe pas\n\n", nom);
        nbErreur++;
    }
   
    fprintf(stderr, "Erreur checkClassDefine :\n");
    fprintf(stderr, "\t> la classe n'existe pas (nom = NIL)\n\n");
    nbErreur++;

    return FALSE;
}


/* Verifie la portee d'une variable */
bool checkPortee(LVarDeclP lvar, char* nom, int *i)
{
    if(nom != NULL)
    {
        int j = 0;
        while(lvar != NIL(LVarDecl) && j < *i)
        {
            if(strcmp(lvar->var->nom, nom) == 0)
            {
                return TRUE;
            }
            j++;
            lvar= lvar->next;
        }

        fprintf(stderr, "Erreur checkPortee :\n");
        fprintf(stderr, "\t> la variable %s n'est pas declaree\n\n", nom);
        nbErreur++;

        return FALSE;
    }

    fprintf(stderr, "Erreur checkPortee :\n");
    fprintf(stderr, "\t> la variable n'est pas declaree (nom = NIL)\n\n");
    nbErreur++;

    return FALSE;
}


/* verification contextuelle d'un bloc */
bool checkBlocMain(TreeP bloc, ClasseP classe, MethodeP methode, int* i)
{
    bool check = TRUE;

    if(bloc != NIL(Tree))
    {
        LVarDeclP tmp = NIL(LVarDecl);

        switch(bloc->op)
        {
            case YCONT :
                /* ajoute dans l'environnement la liste de declaration, si une declaration est fausse addEnv renvoie false */
                tmp = makeLParam(getChild(bloc, 0));

                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe, i) && check;

                check = checkBlocMain(getChild(bloc, 1), classe, methode, i) && check;

                /* met a jour l'env */
                removeEnv(*i, i);
                break;
            
            case LINSTR :
                /* verification d'une liste d'instructions */
                check = checkBlocMain(getChild(bloc, 0), classe, methode, i) && check;
                check = checkBlocMain(getChild(bloc, 1), classe, methode, i) && check;
                break;

            case YITE :
                /* verification d'un if then else */
                /* TODO : l'expression doit etre un bool */
                check = checkExpr(getChild(bloc, 0), classe, methode, i) && check;  
                check = checkBlocMain(getChild(bloc, 1), classe, methode, i) && check;
                check = checkBlocMain(getChild(bloc, 2), classe, methode, i) && check;
                break;

            case EAFF :
                /* verification d'une affectation */
                /* verification de la selection */
                check = checkSelection(getChild(bloc, 0), classe, methode, i) && check;

                /* verification de l'expression */
                check = checkExpr(getChild(bloc, 1), classe, methode, i) && check;

                /* verification de l'affectation */
                VarDeclP var = getVarSelection(getChild(bloc, 0), classe, methode);
                if(var != NIL(VarDecl))
                    check = checkAff(var, getChild(bloc, 1), classe, methode, i) && check;
                break;

            case YEXPR :
                /* verification d'une expression */
                check = checkExpr(getChild(bloc, 0), classe, methode, i) && check;  
                break;

            case YRETURN :
                /* TODO : result doit etre defini ou non avant de faire un return ? */
                break;

            default :
                fprintf(stderr, "Erreur etiquette dans checkBlocMain :\n");
                fprintf(stderr, "\t> etiquette %d inconnu\n\n", bloc->op);
                check = FALSE;
                break;
        }
    }

    return check;
}


/* retourne le type de retour d'une methode */
ClasseP getTypeMethode(char* nom, ClasseP classe)
{
	ClasseP tmpClasse = classe;

	if(tmpClasse != NIL(Classe))
	{
		LMethodeP tmpLMethodes = classe->lmethodes;
		while(tmpLMethodes != NIL(LMethode))
		{
			if(strcmp(nom, tmpLMethodes->methode->nom)==0)
			{
				return tmpLMethodes->methode->typeDeRetour;
			}
			tmpLMethodes = tmpLMethodes->next;
		}
		return getTypeMethode(nom,classe->superClasse);
	}
	else
	{
		return NIL(Classe);
	}

}


/* verifie la validite d'une expression */
bool checkExpr(TreeP tree, ClasseP classe, MethodeP methode, int* i)
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
                    check = checkPortee(env->env, nom, i) && check;
                break;

            case Classname :
                /* verification d'un nom de classe ou d'objet */
                nom = tree->u.str;
                if(strcmp(nom, "this") != 0 && strcmp(nom, "super") != 0 && strcmp(nom, "result") != 0)
                    check = checkClassDefine(nom) && check;
                break;

            case Chaine :
            case Cste :
                break;
        
            case ADD :
                /* TODO : verifier pour le cas unaire */
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
                check = checkExpr(getChild(tree, 0), classe, methode, i) && check;
                check = checkExpr(getChild(tree, 1), classe, methode, i) && check;

                /* verifie que les types des deux expressions sont des Integer */
                typeG = getType(getChild(tree, 0), classe, methode);
                typeD = getType(getChild(tree, 1), classe, methode);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if(!(strcmp(typeG->nom, "Integer") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [op arithmetique] :\n");
                        fprintf(stderr, "\t> l'operande gauche %s n'est pas un Integer\n\n", typeG->nom);
                        nbErreur++;
                        check = FALSE;             
                    }

                    if((!strcmp(typeD->nom, "Integer") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [op arithmetique] :\n");
                        fprintf(stderr, "\t> l'operande droit %s n'est pas un Integer\n\n", typeD->nom);
                        nbErreur++;
                        check = FALSE;             
                    }
                }
                else
                {
                    fprintf(stderr, "Erreur verification d'une expression [op arithmetique] :\n");
                    fprintf(stderr, "\t> type inconnu\n\n");
                    nbErreur++;
                    check = FALSE;
                }
                break;

            case USUB :
                /* verifie l'expression suivant le moins unaire */
                check = checkExpr(getChild(tree, 1), classe, methode, i) && check; 

                /* verifie le type de l'expression */
                typeD = getType(getChild(tree, 1), classe, methode);
                if(typeD != NIL(Classe))
                {
                    if((!strcmp(typeD->nom, "Integer") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [- unaire] :\n");
                        fprintf(stderr, "\t> l'operande droit %s n'est pas un Integer\n\n", typeD->nom);
                        nbErreur++;
                        check = FALSE;             
                    }
                }
                else
                {
                    fprintf(stderr, "Erreur verification d'une expression [- unaire] :\n");
                    fprintf(stderr, "\t> type inconnu\n\n");
                    nbErreur++;
                    check = FALSE;
                }
                break;

            case CONCAT :
                /* verification d'une concatenation */
                /* verifie que les deux expressions sont correctes */
                check = checkExpr(getChild(tree, 0), classe, methode, i) && check;
                check = checkExpr(getChild(tree, 1), classe, methode, i) && check;

                /* verifie que les types des expressions sont des String */
                typeG = getType(getChild(tree, 0), classe, methode);
                typeD = getType(getChild(tree, 1), classe, methode);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if(!(strcmp(typeG->nom, "String") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [concatenation] :\n");
                        fprintf(stderr, "\t> le fils gauche %s n'est pas un String\n\n", typeG->nom);
                        nbErreur++;
                        check = FALSE;             
                    }

                    if((!strcmp(typeD->nom, "String") == 0))
                    {
                        fprintf(stderr, "Erreur verification d'une expression [concatenation] :\n");
                        fprintf(stderr, "\t> le fils droit %s n'est pas un String\n\n", typeD->nom);
                        nbErreur++;
                        check = FALSE;             
                    }
                }
                else
                {
                    fprintf(stderr, "Erreur verification d'une expression [concatenation] :\n");
                    fprintf(stderr, "\t> type inconnu\n\n");
                    nbErreur++;
                    check = FALSE;
                }
                break;

            case ECAST :
                /* verification d'un cast */
                check = checkExpr(getChild(tree, 0), classe, methode, i) && check;
                check = checkExpr(getChild(tree, 1), classe, methode, i) && check;
                check = checkCast(getClassePointer(getChild(tree, 1)->u.str), getChild(tree, 0)->u.str, classe) && check; 
                break;

            case EINST :
                /* verification d'une allocation */
                /* verification du classname du cast */
                check = checkExpr(getChild(tree, 0), classe, methode, i) && check;

                /* verification de la liste d'expressions optionnelles */
                check = checkExpr(getChild(tree, 1), classe, methode, i) && check;

                /* verification de l'allocation */
                ClasseP tmp = getType(getChild(tree, 0), classe, methode);
                if(tmp != NIL(Classe))
                    check = checkMethodes(tmp, tmp->nom, getChild(tree, 1)) && check;
                break;

            case SELEXPR :      
                /* verification d'une selection */
                check = checkSelection(tree, classe, methode, i) && check;
                break;

            case EENVOI :
                /* verification d'un envoi */
                check = checkExpr(getChild(tree, 0), classe, methode, i) && check;
                check = checkMethodes(getType(getChild(tree, 0), classe, methode), getChild(getChild(tree, 1), 0)->u.str, getChild(getChild(tree, 1), 1)) && check;
                break;

            case YLEXPR :
                /* verification d'une liste d'expressions */ 
                check = checkExpr(getChild(tree, 0), classe, methode, i) && check;
                check = checkExpr(getChild(tree, 1), classe, methode, i) && check;
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
bool checkSelection(TreeP selection, ClasseP classe, MethodeP methode, int* i)
{
    bool check = TRUE;

    if(selection->op == Id)         /* cas : Id */
    {
        if(strcmp(selection->u.str, "this") != 0 && strcmp(selection->u.str, "super") != 0 && strcmp(selection->u.str, "result") != 0)
            check = checkPortee(env->env, selection->u.str, i) && check;
        
        if(methode == NIL(Methode) && strcmp(selection->u.str, "result") == 0)
        {
            fprintf(stderr, "Erreur selection\n");
            fprintf(stderr, "\t> la variable result est interdite dans un constructeur\n\n");
            nbErreur++;
            check = FALSE;            
        }
    }

    else                            /* cas : Expr '.' Id */
    {
        /* verification de l'expression */
        check = checkExpr(getChild(selection, 0), classe, methode, i) && check;

        if(check)
        {
            ClasseP tmp = getType(getChild(selection, 0), classe, methode);
            char* nom = getChild(selection, 1)->u.str;

            if(tmp != NIL(Classe))
            {
                /* soit classe.attribut dans une methode de other, si other n'herite pas de classe, alors 
                classe.attribut est faux car l'attribut n'est pas visible dans other */
                if(checkHeritageClasse(classe, tmp->nom))
                {
                    LVarDeclP liste = tmp->lchamps;

                    while(liste != NIL(LVarDecl))
                    {
                        if(strcmp(liste->var->nom, nom) == 0)
                        {
                            return check;
                        }

                        liste = liste->next;
                    }

                    if(!check)
                    {
                        fprintf(stderr, "Erreur selection\n");
                        fprintf(stderr, "\t> %s n'est pas un champs de la classe %s\n\n", nom, tmp->nom);
                        nbErreur++;
                    }
                }
                else
                {
                    check = FALSE;
                    fprintf(stderr, "Erreur selection\n");
                    if(classe != NIL(Classe))
                    {
                        fprintf(stderr, "\t> les champs de %s ne sont pas visibles dans %s\n\n", tmp->nom, classe->nom);
                    }
                    else
                    {
                        fprintf(stderr, "\t> les champs de %s ne sont pas visibles dans le main\n\n", tmp->nom);
                    }
                    nbErreur++;
                }
            }
        }
    }

    return check;
}


/* retourne le nom du type d'une expression correcte ou NIL */
ClasseP getType(TreeP expr, ClasseP classe, MethodeP methode)
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
                /* retourne le type de l'id : traite egalement les variables reservees */
                if(strcmp(expr->u.str, "this") == 0)
                {
                    if(classe != NIL(Classe))
                    {
                        type = classe;
                    }	
                    else
                    {
                        fprintf(stderr, "Erreur de type :\n");
                        fprintf(stderr, "\t> usage incorrect de this\n\n");
                        nbErreur++;
                    }
                }
                else if(strcmp(expr->u.str, "result") == 0)
                {
                    if(methode != NIL(Methode) && methode->typeDeRetour != NIL(Classe))
                    {
                        type = methode->typeDeRetour;
                    }
                	else
                	{
                		fprintf(stderr, "Erreur de type :\n");
                        fprintf(stderr, "\t> usage incorrect de result\n\n");
                        nbErreur++;
                	}
                }
                else if(strcmp(expr->u.str, "super") == 0)
                {
                	if(classe != NIL(Classe) && classe->superClasse != NIL(Classe))
                	{
                		type = classe->superClasse;
                	}
                	else
                	{
                		fprintf(stderr, "Erreur de type :\n");
                        fprintf(stderr, "\t> usage incorrect de super\n\n");
                        nbErreur++;
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
                /* TODO : verifier avec le cas unaire */
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
                typeG = getType(getChild(expr, 0), classe, methode);
                typeD = getType(getChild(expr, 1), classe, methode);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if((strcmp(typeG->nom, typeD->nom) == 0) && (strcmp(typeG->nom, "Integer") == 0))
                    {
                        type = typeG;           
                    }
                }
                break;

            case USUB :
                /* retourne Integer si l'operation unaire est correcte */
                typeD = getType(getChild(expr, 1), classe, methode); 
                if(strcmp(typeD->nom, "Integer") == 0)
                    type = typeD;
                break;

            case CONCAT : 
                /* retourne String si la concatenation est entre deux String */
                typeG = getType(getChild(expr, 0), classe, methode);
                typeD = getType(getChild(expr, 1), classe, methode);

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if((strcmp(typeG->nom, typeD->nom) == 0) && (strcmp(typeG->nom, "String") == 0))
                    {
                        type = typeG;           
                    }
                }
                break;

            case ECAST :
                /* retourne le type du cast si le cast est entre deux types existants */
                typeG = getType(getChild(expr, 0), classe, methode);
                typeD = getType(getChild(expr, 1), classe, methode);
                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    type = typeG;           
                }
                break;

            case EINST :
                /* retourne le type de l'allocation */
                type = getType(getChild(expr, 0), classe, methode);
                break;

            case SELEXPR :    
                /* retourne le type de la selection ou de l'id */
                if(expr->nbChildren == 2) /* cas expr '.' id */
                {
                    /* on recupere la classe de l'expr */
                    typeG = getType(getChild(expr, 0), classe, methode);
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
                    }
                }
                else /* cas id */
                {
                    type = getType(getChild(expr, 0), classe, methode);
                }
                break;

            case EENVOI :
                /* retourne le type de la methode */
                typeG = getType(getChild(expr, 0), classe, methode);
                
                if(typeG != NIL(Classe))
                {
                    MethodeP methode = getMethodePointer(typeG, getChild(getChild(expr, 1), 0)->u.str);
                    if(methode != NIL(Methode))
                    {
                        type = methode->typeDeRetour;
                    }
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
bool setEnvironnementType(LVarDeclP var, ClasseP classe, MethodeP methode, int* i)               
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
                    check = checkAff(tmp->var, tmp->var->exprOpt, classe, methode, i);
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
bool checkBlocClasse(TreeP tree, ClasseP classe, MethodeP methode, int* i)
{
    bool check = TRUE;

    if(tree != NIL(Tree))
    {
        LVarDeclP tmp = NIL(LVarDecl);
        ClasseP superClasse = NIL(Classe);

        switch(tree->op)
        {
            case YLCLASS :
                /* parcours de la liste de classe ou d'objet */
                check = checkBlocClasse(getChild(tree, 0), classe, methode, i) && check;
                check = checkBlocClasse(getChild(tree, 1), classe, methode, i) && check;
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
                        check = addEnv(tmp, classe, i) && check;
                    }
                    else
                    {
                        fprintf(stderr, "Erreur d'heritage dans checkBlocClasse :\n");
                        fprintf(stderr, "\t> la classe %s est inconnu\n\n", getChild(extends, 0)->u.str);
                        nbErreur++;
                    }
                }

                /* ajoute les champs de la classe dans l'env */
                tmp = classe->lchamps;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe, i) && check;

                /* ajoute les parametres de la classe dans l'env pour faire la verification du constructeur */
                tmp = classe->lparametres;
                check = verifLParam(tmp) && check;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe, i) && check;
           
                /* verification du constructeur de la classe */ 
                TreeP constructeur = getChild(tree, 3);
                check = checkBlocMain(constructeur, classe, NIL(Methode), i) && check;

                /* retire les parametres de la classe de l'env */
                int tailleLParam = getTailleListeVarDecl(tmp);
                removeEnv(tailleLParam, i);

                /* verifie le blocObj de la classe */
                check = checkBlocClasse(getChild(tree, 4), classe, methode, i) && check; 
                
                /* reset l'env */
                removeEnv(env->taille, i);
                break;

            case YOBJ :
                /* verifie qu'un objet est correct */
                classe = getClassePointer(getChild(tree, 0)->u.str);

                /* ajoute les champs de l'objet dans l'env */
                tmp = classe->lchamps;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe, i) && check;

                /* verifie le blocObj de l'objet */
                check = checkBlocClasse(getChild(tree, 1), classe, methode, i) && check; 

                /* reset l'env */
                removeEnv(env->taille, i);
                break;

            case LDECLMETH :
                /* verification d'une liste de declarations */
                check = checkBlocClasse(getChild(tree, 0), classe, methode, i) && check;
                check = checkBlocClasse(getChild(tree, 1), classe, methode, i) && check;
                break; 

            case YDECLC :
                /* les champs d'une classe ou d'un objet sont deja ajoutes dans l'env */
                break;

            case DMETHODEL :
                /* verification d'une methode : OverrideOpt DEF Id '(' LParamOpt ')' ':' Classname AFF Expr*/
                /* met a jour la variable methode */
                methode = getMethodePointer(classe, getChild(tree, 1)->u.str);

                /* ajoute dans l'env les parametres de la methode */
                tmp = makeLParam(getChild(tree, 2));
                check = verifLParam(tmp) && check;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe, i) && check;

                /* verifie le type de retour de la methode */
                check = checkClassDefine(getChild(tree, 3)->u.str) && check;

                /* verifie l'expression associee a la methode */
                check = checkExpr(getChild(tree, 4), classe, methode, i) && check;

                /* met a jour l'env */
                removeEnv(*i, i);
                break;

            case DMETHODE :
                /* verification d'une methode : OverrideOpt DEF Id '(' LParamOpt ')' TypeCOpt IS Bloc */
                /* TODO : mettre dans cette partie la verification d'une declaration d'une methode 
                * reste encore a mettre : override correct (attention entre classe et objet), nom correct (pas de surcharge) 
                */

                /* met a jour la variable methode */
                methode = getMethodePointer(classe, getChild(tree, 1)->u.str);

                /* ajoute dans l'env les parametres de la methode */
                tmp = makeLParam(getChild(tree, 2));
                check = verifLParam(tmp) && check;
                if(tmp != NIL(LVarDecl))
                    check = addEnv(tmp, classe, i) && check;

                /* verifie le type de retour de la methode */
                TreeP type = getChild(tree, 3);
                if(type != NIL(Tree))
                    check = checkClassDefine(type->u.str) && check;

                /* verifie le bloc associe a la methode */
                check = checkBlocMain(getChild(tree, 4), classe, methode, i) && check;

                /* met a jour l'env */
                removeEnv(*i, i);
                break;

            default :
                fprintf(stderr, "Erreur etiquette dans checkBlocClasse :\n");
                fprintf(stderr, "\t> etiquette %d inconnu\n\n", tree->op);
                check = FALSE;
                break;
        }
    }

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
bool addEnv(LVarDeclP var, ClasseP classe, int* i)
{
    bool check = TRUE;
    
    if(var != NIL(LVarDecl))
    {
        LVarDeclP tmp = var;
        while(tmp != NIL(LVarDecl))
        {
            check = addVarEnv(tmp->var, classe, i) & check;
            tmp = tmp->next;
        }
    }

    return check;
}


/* ajoute une variable dans env et verifie son affection opt ainsi que son type */
bool addVarEnv(VarDeclP var, ClasseP classe, int* i)
{
    bool check = TRUE;

    if(var != NIL(VarDecl))
    {
        if(strcmp(var->nom, "this") != 0 && strcmp(var->nom, "super") && strcmp(var->nom, "result"))
        {
            ScopeP newEnv = NEW(1, Scope);

            LVarDeclP tmp = NEW(1, LVarDecl);
            tmp->var = var;
            tmp->next = NIL(LVarDecl);

            check = setEnvironnementType(tmp, classe, NIL(Methode), i);

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

            *i = *i + 1;
        }
        else
        {
            fprintf(stderr, "Erreur de declaration de variable\n");
            fprintf(stderr, "\t> la variable %s est un id reserve\n\n", var->nom);
            nbErreur++;
        }
    }

    return check;
}


/* retire n variables de l'env */
void removeEnv(int n, int *i)
{
    if(env->taille < n)
    {
        fprintf(stderr, "Erreur removeEnv\n");
        return;
    }

    if(n > 0)
    {
        int j;
        LVarDeclP tmp = env->env;
        for(j = 0; j < n; j++)
        {
            tmp = tmp->next;
        }

        env->env = tmp;
        env->taille = env->taille - n;

        *i = *i - n;
    }
}


/* verifie une affectation */
bool checkAff(VarDeclP var, TreeP expr, ClasseP classe, MethodeP methode, int* i)
{
    bool check = checkExpr(expr, classe, methode, i);

    if(check)
    {
        VarDeclP op = NIL(VarDecl);
        ClasseP tmp = NIL(Classe);

        switch(expr->op)
        {
            case Id :
                op = getVarSelection(expr, classe, methode);
                if(op != NIL(VarDecl))
                {
                    if(*(op->isDefini))
                    {
                        tmp = getTypeId(op->nom);
                        if(tmp != NIL(Classe) && var != NIL(VarDecl))
                        {
                            if(strcmp(var->type->nom, tmp->nom) != 0 && !checkHeritageClasse(tmp, var->type->nom))
                            {
                                check = FALSE;
                                fprintf(stderr, "Erreur verification affectation\n");
                                fprintf(stderr, "\t> impossible d'affecter un %s a un %s\n\n", var->type->nom, tmp->nom);
                                nbErreur++;
                            }
                            else
                            {
                                *(var->isDefini) = TRUE;
                            }
                        }
                        else
                        {
                            check = FALSE;
                            fprintf(stderr, "Erreur verification affectation\n");
                            fprintf(stderr, "\t> affectation impossible\n\n");
                            nbErreur++;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Erreur verification affectation\n");
                        fprintf(stderr, "\t> la variable %s est indefini\n\n", op->nom);
                        nbErreur++;
                    }
                }
                break;

            case SELEXPR :
                op = getVarSelection(expr, classe, methode);
                if(op != NIL(VarDecl))
                {
                    if(*(op->isDefini))
                    {
                        tmp = getTypeId(op->nom);
                        if(tmp != NIL(Classe) && var != NIL(VarDecl))
                        {
                            if(strcmp(var->type->nom, tmp->nom) != 0 && !checkHeritageClasse(tmp, var->type->nom))
                            {
                                check = FALSE;
                                fprintf(stderr, "Erreur verification affectation\n");
                                fprintf(stderr, "\t> impossible d'affecter un %s a un %s\n\n", var->type->nom, tmp->nom);
                                nbErreur++;
                            }
                            else
                            {
                                *(var->isDefini) = TRUE;
                            }
                        }
                        else
                        {
                            check = FALSE;
                            fprintf(stderr, "Erreur verification affectation\n");
                            fprintf(stderr, "\t> affectation impossible\n\n");
                            nbErreur++;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Erreur verification affectation\n");
                        fprintf(stderr, "\t> la variable %s est indefini\n\n", op->nom);
                        nbErreur++;
                    }
                }
                break;

            default :
                tmp = getType(expr, classe, methode);
                if(tmp != NIL(Classe) && var != NIL(VarDecl))
                {
                    if(strcmp(var->type->nom, tmp->nom) != 0 && !checkHeritageClasse(tmp, var->type->nom))
                    {
                        check = FALSE;
                        fprintf(stderr, "Erreur verification affectation\n");
                        fprintf(stderr, "\t> impossible d'affecter un %s a un %s\n\n", var->type->nom, tmp->nom);
                        nbErreur++;
                    }
                    else
                    {
                        *(var->isDefini) = TRUE;
                    }
                }
                else
                {
                    check = FALSE;
                    fprintf(stderr, "Erreur verification affectation\n");
                    fprintf(stderr, "\t> affectation impossible\n\n");
                    nbErreur++;
                }
                break;
        }
    }
    
    return check;
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
            if(strcmp(tmp->var->nom, "this") != 0 && strcmp(tmp->var->nom, "super") && strcmp(tmp->var->nom, "result"))
                i++;
            tmp = tmp->next;
        }
    }

    return i;
}


/* retourne la variable selectionne ou NIL */
VarDeclP getVarSelection(TreeP selection, ClasseP classe, MethodeP methode)
{
    if(selection->op == Id)
    {
        LVarDeclP liste = env->env;
        char* nom = selection->u.str;

        if(strcmp(nom, "result") != 0)
        {
            while(liste != NIL(LVarDecl))
            {
                if(strcmp(liste->var->nom, nom) == 0)
                {
                    return liste->var;
                }

                liste = liste->next;
            }
        }
        else /* result */
        {
            if(methode != NIL(Methode) && methode->typeDeRetour != NIL(Classe) && strcmp(methode->typeDeRetour->nom, "Void") != 0)
            {
                VarDeclP result = makeVarDecl("result", methode->typeDeRetour->nom, NIL(Tree));
                return result;
            }
            /*
            else
            {
                fprintf(stderr, "Erreur selection\n");
                fprintf(stderr, "\t> usage incorrect de result\n\n");
                nbErreur++;
            }
            */
        }
    }
    else
    {
        ClasseP tmp = getType(getChild(selection, 0), classe, methode);
        char* nom = getChild(selection, 1)->u.str;

        if(tmp != NIL(Classe))
        {
            LVarDeclP liste = tmp->lchamps;

            while(liste != NIL(LVarDecl))
            {
                if(strcmp(liste->var->nom, nom) == 0)
                {
                    return liste->var;
                }

                liste = liste->next;
            }
        }
    }

    fprintf(stderr, "Erreur selection\n");
    fprintf(stderr, "\t> selection introuvable\n\n");
    nbErreur++;
    return NIL(VarDecl);    
}


/* verifie qu'une liste de parametres possede toutes les expressions par defaut a la fin */
bool verifLParam(LVarDeclP lparam)
{
    bool check = FALSE;

    LVarDeclP liste = lparam;

    while(liste != NIL(LVarDecl))
    {
        if(liste->var->exprOpt != NIL(Tree))
        {
            check = TRUE;
        }

        if(check && !(liste->var->exprOpt != NIL(Tree)))
        {
            fprintf(stderr, "Erreur de declaration de parametres\n");
            fprintf(stderr, "\t> les parametres avec une expressions par defaut ne sont pas tous declare a la fin\n\n");
            nbErreur++;
            return FALSE;
        }

        liste = liste->next;
    }

    return TRUE;
} 


























bool checkHeritageClasse(ClasseP classe, char* nom)
{
	if(classe != NIL(Classe))
	{
		if(strcmp(classe->nom, nom) == 0)
	    {
	        return TRUE; 
	    }
	    else
	    {
	        if(classe->superClasse != NULL)
	        {
	            return checkHeritageClasse(classe->superClasse, nom);
	        }
	        else
	        {
	            return FALSE;
	        }
	    }
	}
	else
	{
		return FALSE;
	}
}

bool checkArguments(LParamP larg, LParamP largbis)
{
    bool retour = TRUE;
    LParamP larg1 = larg;
    LParamP larg2 = largbis;
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

/*Renvoie faux si les argument sont mal redefinis dans un override*/
bool CheckArgumentOverride(LParamP nvlarg, LParamP larg)
{
    LParamP anc = larg;
    LParamP nv = nvlarg;

    while(anc != NIL(LParam) && nv != NIL(LParam))
    {
        if(strcmp(anc->var->nom,nv->var->nom)==0 && anc->var->type == nv->var->type)
        {
            return !(anc->var->exprOpt == NIL(Tree) && nv->var->exprOpt == NIL(Tree));
        }
        else
        {
            return FALSE;
        }
        anc = anc->next;
        nv = nv->next;
    }
    return (anc == NIL(LParam ) && nv == NIL(LParam));
}

/*Renvoie true si pas de probleme d'override*/
bool checkOverrideMethode(ClasseP classe, char* nom, LParamP larg, bool isOverride, ClasseP typeDeRetour)
{
    if(classe != NIL(Classe))
    {
        LMethodeP tmpMethodes = classe->lmethodes;
        while(tmpMethodes != NIL(LMethode))
        {
            if(strcmp(tmpMethodes->methode->nom,nom) == 0 )
            {
                if(!isOverride)
                {
                    printf("| Erreur Override | Rajouter un override dans la fonction %s de ",nom );
                    nbErreur++;
                    return FALSE;
                }
                else
                {
                    bool b = TRUE;
                    if(tmpMethodes->methode->typeDeRetour != typeDeRetour)
                    {
                        printf("| Erreur Override | Le type de retour de la méthode %s sont différents dans la classe %s et ",nom,classe->nom);
                        nbErreur++; 
                        b = FALSE;
                    }
                    else
                    {
                        LParamP tmp = larg;
                        b = CheckArgumentOverride(tmp,tmpMethodes->methode->lparametres);
                        if(!b)
                        {
                            printf("| Erreur Override | Erreur dans les paramètres de la méthode %s dans la classe %s et ",nom,classe->nom);
                            nbErreur++; 
                        }
                    }
                    return b;
                }
            }
            tmpMethodes = tmpMethodes->next;
        }
        if(classe->superClasse != NIL(Classe))
        {
            return checkOverrideMethode(classe->superClasse, nom, larg, isOverride, typeDeRetour);
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
                if(tmpMethodes->methode->override && !checkOverrideMethode(tmpClasses->classe->superClasse, tmpMethodes->methode->nom, tmpMethodes->methode->lparametres,tmpMethodes->methode->override, tmpMethodes->methode->typeDeRetour))
                {
                    if(tmpClasses->classe->superClasse == NIL(Classe)) printf("| Erreur Override | La classe mère n'existe pas alors qu'il y a un override dans ");
                    printf("la classe %s \n\n", tmpClasses->classe->nom);
                    b =  FALSE;
                }
                if(!tmpMethodes->methode->override && checkOverrideMethode(tmpClasses->classe->superClasse, tmpMethodes->methode->nom, tmpMethodes->methode->lparametres,tmpMethodes->methode->override, tmpMethodes->methode->typeDeRetour))
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
        fprintf(stderr, "\t> classe inexistante\n\n");
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
                while(compatible && methLParam != NIL(LParam) && methLParam->var->exprOpt == NIL(Tree))
                {
                    if(tmplparam != NIL(Tree))
                    {
                        if(tmplparam->op == YLEXPR)
                        {
                            if(checkHeritageClasse(getType(getChild(tmplparam, 0), NIL(Classe), NIL(Methode)), methLParam->var->type->nom))
                            {
                                methLParam = methLParam->next;
                                tmplparam = getChild(tmplparam,1);
                            }
                            else
                            {
                                compatible = FALSE;
                            }
                        }
                        else
                        {
                            if(checkHeritageClasse(getType(tmplparam, NIL(Classe), NIL(Methode)), methLParam->var->type->nom))
                            {
                                methLParam = methLParam->next;
                                tmplparam = NIL(Tree);
                            }
                            else
                            {
                                compatible = FALSE;
                            }
                        }
                    }
                    else
                    {
                        compatible = FALSE;
                    }
                }

                while(compatible && tmplparam != NIL(Tree))
                {
                    if(methLParam == NIL(LParam))
                    {
                        compatible = FALSE;
                    }
                    else 
                    {
                        if(tmplparam->op == YLEXPR)
                        {
                            if(getType(getChild(tmplparam, 0), NIL(Classe), NIL(Methode)) != NIL(Classe) && checkHeritageClasse(getType(getChild(tmplparam, 0), NIL(Classe), NIL(Methode)), methLParam->var->type->nom))
                            {
                                methLParam = methLParam->next;
                                tmplparam = getChild(tmplparam,1);
                            }
                            else
                            {
                                methLParam = methLParam->next;
                            }
                        }
                        else
                        {
                            if(getType(tmplparam, NIL(Classe), NIL(Methode)) != NIL(Classe) && checkHeritageClasse(getType(tmplparam, NIL(Classe), NIL(Methode)), methLParam->var->type->nom))
                            {
                                methLParam = methLParam->next;
                                tmplparam = NIL(Tree);
                            }
                            else
                            {
                                methLParam = methLParam->next;
                            }
                        }
                    }
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
            fprintf(stderr, "\t> %s n'est pas une methode de %s ou n'a pas les bon parametres\n\n", nom, classe->nom);
            nbErreur++;
            /* fprintf(stderr, "erreur ligne : %d\n", yylineno); */
        }
    } 
    else
    {
        fprintf(stderr, "Erreur verification methode :\n");
        fprintf(stderr, "\t> le type de l'envoi n'existe pas pour la methode %s\n\n", nom);
        nbErreur++;
    }
        
    return check;
}