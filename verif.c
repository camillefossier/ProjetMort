#include <string.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char*);
extern void setError(int code);

extern LClasseP lclasse;
extern ObjetP lobjet;
extern ScopeP env;


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


/* verifie l'extistance d'une classe */
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
   
    fprintf(stderr, "Erreur checkClassDefine pour la classe : %s\n",nom);
    return FALSE;
}


/*
*Verifie la portee d'une variable nom*
*/
bool checkPortee(VarDeclP lvar, char* nom)
{
    if(nom != NULL)
    {
        while(lvar != NIL(VarDecl))
        {
            if(strcmp(lvar->nom, nom)==0)
            {
                return TRUE;
            }
            lvar= lvar->next;
        }
    }
    fprintf(stderr, "Erreur checkPortee pour la var : %s\n", nom);
    return FALSE;
}


bool checkBlocMain(TreeP bloc)
{
    int* i = NEW(1, int);
    *i = 0;

    bool check = TRUE;

    if(bloc != NIL(Tree))
    {
        VarDeclP tmp = NIL(VarDecl);

        switch(bloc->op)
        {
            case YCONT :
              tmp = makeLParam(getChild(bloc, 0), i);
              if(tmp != NIL(VarDecl))
                addEnv(tmp);
              check = (check && checkBlocMain(getChild(bloc, 1)));
              removeEnv(*i);
              break;
            
            case LINSTR :
              check = (check && checkBlocMain(getChild(bloc, 0)));
              check = (check && checkBlocMain(getChild(bloc, 1)));
              break;

            case YITE :
              check = (check && checkExpr(getChild(bloc, 0)));  
              check = (check && checkBlocMain(getChild(bloc, 1)));
              check = (check && checkBlocMain(getChild(bloc, 2)));
              break;

            case EAFF :
              check = (check && checkBlocMain(getChild(bloc, 1)));
              check = (check && checkExpr(getChild(bloc, 1)));
              break;

            case SELEXPR :
              check = (check && checkSelection(bloc));
              break;

            case Id :
             printf("checkBlocMain Id\n" );
              check = (check && checkPortee(env->env, bloc->u.str));  
              break;  

            case YEXPR :
              check = (check && checkExpr(getChild(bloc, 0)));  
              break;

            case YRETURN :
              break;

            default :
              fprintf(stderr, "Erreur etiquette dans checkBlocMain\n");
              check = FALSE;
              break;
        }
    }

    free(i);
    return check;
}


/* verifie la validite d'une expression */
/* TODO : ne fait que la portee, faut faire le reste */
bool checkExpr(TreeP tree)
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
            printf("checkExpr Id\n");
                nom = tree->u.str;
                check = (check && checkPortee(env->env, nom));
                break;

            case Classname :
                nom = tree->u.str;
                check = (check && checkClassDefine(nom));
                break;

            case Chaine :
            case Cste :
                break;
        
            case ADD :
                /* TODO : verif avec le trucs unaires */
            case SUB :
            case MUL :
            case DIV :    
            case EQ :
            case NE :
            case SUP :
            case SUPE :
            case INF :
            case INFE :

                check = (check && checkExpr(getChild(tree, 0)));
                check = (check && checkExpr(getChild(tree, 1)));

                typeG = getType(getChild(tree, 0));
                typeD = getType(getChild(tree, 1));

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if(!(strcmp(typeG->nom, "Integer") == 0) || (!strcmp(typeD->nom, "Integer") == 0))
                    {
                        check = FALSE;             
                    }
                }
                else
                {
                    check = FALSE;
                }
                
                break;

            case USUB :
                /* TODO unaires*/
                check = (check && checkExpr(getChild(tree, 1))); 
                break;

            case CONCAT :

                check = (check && checkExpr(getChild(tree, 0)));
                check = (check && checkExpr(getChild(tree, 1)));

                typeG = getType(getChild(tree, 0));
                typeD = getType(getChild(tree, 1));

                if(typeG != NIL(Classe) && typeD != NIL(Classe))
                {
                    if(!(strcmp(typeG->nom, "String") == 0) || (!strcmp(typeD->nom, "String") == 0))
                    {
                        check = FALSE;             
                    }
                }
                else
                {
                    check = FALSE;
                }
                

                break;

            case ECAST :

                check = (check && checkExpr(getChild(tree, 0)));
                check = (check && checkExpr(getChild(tree, 1)));
    
                /* TODO : cast */
                check = (check && checkCast(getClassePointer(getChild(tree, 1)->u.str), getChild(tree, 0)->u.str)); 
                break;

            case EINST :

                check = (check && checkExpr(getChild(tree, 0)));
                check = (check && checkExpr(getChild(tree, 1)));
                break;

            case SELEXPR :      

                check = (check && checkSelection(tree));
                break;

            case EENVOI :

                check = (check && checkExpr(getChild(tree, 0)));
                /*check = (check && checkExpr(getChild(tree, 1)));*/

                /* TODO : envoi */
                check = (check && checkMethodes(getType(getChild(tree, 0)), getChild(getChild(tree, 1), 0)->u.str));
                break;

            case YLEXPR : 
                check = (check && checkExpr(getChild(tree, 0)));
                check = (check && checkExpr(getChild(tree, 1)));
                break; 

            default :
                fprintf(stderr, "Erreur etiquette dans checkExpr\n");
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
    return TRUE;
}



/* retourne le nom du type d'une expression correcte ou NIL */
ClasseP getType(TreeP expr)
{
    ClasseP type = NIL(Classe);

    if(expr != NIL(Tree))
    {
        ClasseP typeG;
        ClasseP typeD;

        switch (expr->op)
        {
            case Id :
                type = getTypeId(expr->u.str);
                break;

            case Classname :
                type = getClassePointer(expr->u.str);
                break;

            case Chaine :
                type = getClassePointer("Integer");
                break;

            case Cste :
                type = getClassePointer("String");
                break;
        
            case ADD :
                /* TODO : verif avec le trucs unaires */
            case SUB :
            case MUL :
            case DIV :            
            case EQ :
            case NE :
            case SUP :
            case SUPE :
            case INF :
            case INFE :

                typeG = getType(getChild(expr, 0));
                typeD = getType(getChild(expr, 1));

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
                /* TODO unaires*/
                type = getType(getChild(expr, 1));; 
                break;

            case CONCAT : 

                typeG = getType(getChild(expr, 0));
                typeD = getType(getChild(expr, 1));

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

                typeG = getType(getChild(expr, 0));
                typeD = getType(getChild(expr, 1));

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

                type = getType(getChild(expr, 0));
                break;

            case SELEXPR :      

                if(expr->nbChildren == 2)
                {
                    type = getType(getChild(expr, 1));
                }
                else
                {
                    type = getType(getChild(expr, 0));
                }
                break;

            case EENVOI :

                typeG = getType(getChild(expr, 0));
                
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
                fprintf(stderr, "Erreur etiquette dans getType\n");
                type = NIL(Classe);
                break;
        }
    }

    return type;
}


/* retourne le nom du type d'un id */
ClasseP getTypeId(char* nom)
{
    if(env != NIL(Scope))
    {
        VarDeclP tmp = env->env;
        while(tmp != NIL(VarDecl))
        {
            if(strcmp(nom, tmp->nom) == 0)
            {
                return tmp->type;
            }
            tmp = tmp->next;
        }
    }
    return NIL(Classe);
}  


/* renvoie une methode a partir d'un nom */
MethodeP getMethodePointer(ClasseP classe, char* nom)
{
    LMethodeP tmp = classe->lmethodes;
    while(tmp != NIL(LMethode))
    {
        if(strcmp(tmp->methode->nom,nom)==0)
        {
            return tmp->methode;
        }
        else
        {
            tmp = tmp->next;
        }
    }
    if(classe->superClasse != NIL(Classe))
    {
        return getMethodePointer(classe->superClasse, nom);
    }
    else
    {
        return NIL(Methode);
    }
}




































































bool checkArguments(ParamP larg1, ParamP larg2)
{
    bool retour = TRUE;
    while(larg1 != NIL(Param) && larg2 != NIL(Param))
    {
        if(larg1->type != NIL(Classe) && larg2->type != NIL(Classe) && strcmp(larg1->type->nom, larg2->type->nom) == 0)
        {
            larg1 = larg1->next;
            larg2 = larg2->next;
        }
        else
        {
            retour = FALSE;
        }
    }
    if(larg1 != NIL(Param) || larg2 != NIL(Param)) retour = FALSE;
    return retour;
}

/*Renvoie true si pas de probleme d'override*/
bool checkOverrideMethode(ClasseP classe, char* nom, ParamP larg, bool isOverride)
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
                    }
                    return TRUE;
                }
                else
                {
                    if(!isOverride)
                    {
                        printf("surcharge \n");
                    }
                    else
                    {
                        printf("| Erreur Override | Les paramètres de la méthode %s sont différents dans la classe %s et ",nom,classe->nom);                      
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
        LClasseP tmp= tmpClasses;
        tmpClasses = tmpClasses->next;
        while(tmpClasses != NIL(LClasse))
        {
           if(strcmp(tmpClasses->classe->nom,tmp->classe->nom)==0)     
           {
                printf("Doublon de classe concernant la classe de nom %s\n", tmp->classe->nom);
                return FALSE;
           }
           tmpClasses = tmpClasses->next;
        }
        return checkDoublonClasse(tmpClasses);
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
                printf("Erreur, Boucle d'héritage concernant la classe %s\n", tmpClasses->classe->nom);
                return FALSE;
            }
            tmpSuper = tmpSuper->superClasse;
        }
        tmpClasses = tmpClasses->next;
    }
    return TRUE;
}

/*devrait marche, à tester avec les blocs*/
bool checkCast(ClasseP classe, char* nom)
{
    if(strcmp(classe->nom,nom)==0)
    {
        return TRUE;
    }
    else
    {
        if(classe->superClasse != NULL)
        {
            return checkCast(classe->superClasse,nom);
        }
        else
        {
            return FALSE;
        }
    }
}

/*Fonction utile pour un envoi : on regarde si la méthode existe dans la classe de l'objet*/
bool checkMethodes(ClasseP classe, char* nom)
{
    LMethodeP tmp = classe->lmethodes;
    while(tmp != NIL(LMethode))
    {
        if(strcmp(tmp->methode->nom,nom)==0)
        {
            return TRUE;
        }
        else
        {
            tmp = tmp->next;
        }
    }
    if(classe->superClasse != NIL(Classe))
    {
        return checkMethodes(classe->superClasse, nom);
    }
    else
    {
        return FALSE;
    }

    /* TODO : ajouter la verif des parametres */
}

bool checkTypeAff(VarDeclP var, TreeP expr)
{
    ClasseP tmp = getType(expr);
    if(tmp != NIL(Classe))
    {
        return (var != NIL(VarDecl) && strcmp(var->type->nom, tmp->nom)==0);
    }

    return FALSE;
}

bool checkLArg(VarDeclP lvar)
{
    bool retour = TRUE;
    VarDeclP temp = lvar;
    while(temp != NIL(VarDecl)) {
        if(temp->exprOpt != NIL(Tree)) {
            retour = checkTypeAff(temp,temp->exprOpt);
        }
        temp = temp->next;
    }
    return retour;
}

/*
*
Pour descendre environnement d'une hauteur  TODO : update environnement (ajouter des lvar)
*/
/*void transmetEnv(TreeP tree)
{
    int i;
    for(i=0; i<tree->nbChildren ; i++)
    {
        getChild(tree,i)->u.lvar = tree->u.lvar;    
    }
}*/


/*
*Verifie si le nom est bien attribue a une classe defini*
**/

/*bool checkClassDefine(LClasseP lenv_classe, char* nom)
{
    if(nom != NULL)
    {
        ClasseP env_classe = lenv_classe->classe;
        while(env_classe != NIL(Classe))
        {
            if(strcmp(env_classe->nom, nom)==0)
            {
                return TRUE;
            }
        env_classe = lenv_classe->next->classe;
        }
    }
   
    fprintf(stderr, "Erreur checkClassDefine pour la classe : %s\n",nom);
    return FALSE;
}*/


/*
*Verifie qu'une expression**
*/
/*
char *checkExpr(TreeP tree, ClasseP classes, VarDeclP env)
{
    if(tree != NIL(Tree))
    {
        char *type;
        char *typeD;
        switch (tree->op)
        {
            case Id :
                type = tree->u.str;
                if(checkPortee(env, type))
                {
                    while(env != NIL(VarDecl))
                    {
                        if(strcmp(type, env->name) == 0)
                        {*
                            *return env->type->type->nom;**
                        }
                        env = env->next;
                    }
                }
                fprintf(stderr, "ERREUR checkExpr id : %s\n", type);
                return NULL;
            case Classname :
                type = tree->u.str;
                if(checkClassDefine(classes, type))
                    return type;
                fprintf(stderr, "ERREUR checkExpr Classname : %s\n", type);
                return NULL;
            case Chaine :
                return "String";
            case Cste :
                return "Integer";
            case INTC :
                return "Integer";
            case STRINGC :
                return "String";
            case VOIDC :    
                return "Void";
            case ADD :
            case SUB :
            case MUL :
            case DIV :    
            case EQ :
            case NE :
            case SUP :
            case SUPE :
            case INF :
            case INFE :
                type = checkExpr(getChild(tree, 0), classes, env);
                typeD = checkExpr(getChild(tree, 1), classes, env);
                if(strcmp(type, "Integer") == 0
                    && strcmp(type, typeD) == 0 )
                {
                    return "Integer";              
                }
                
                fprintf(stderr, "ERREUR getType operation arithmetique : %s ou %s \n",type,typeD);
                return NULL;
            case CONCAT :
                type = checkExpr(getChild(tree, 0), classes, env);
                typeD = checkExpr(getChild(tree, 1), classes, env);
                if(strcmp(type, "String") == 0
                    && strcmp(type, typeD) == 0 )
                {
                    return "String";                    
                }
                
                fprintf(stderr, "ERREUR getType concat : %s ou %s \n",type,typeD);
                return NULL;
            case ECAST :
                 TODO 
                break;
            case EINST :
                type = checkExpr(getChild(tree, 0), classes, env);
                if(checkClassDefine(classes, type))
                {
                    return type;
                }
                fprintf(stderr, "ERREUR getType new Type : %s\n",type);
                return NULL;
            case SELEXPR :                          
            case EENVOI :
                return checkExpr(getChild(tree, 1), classes, env);
            case YLEXPR : 
            case METHOD :
                return checkExpr(getChild(tree, 0), classes, env);       
            default :
                fprintf(stderr, "Erreur dans getType");
                return NULL;
        }
    }
    return NULL;
}
*/

/*
bool checkTypeAff(VarDeclP var, char* nom, ExpressionP expr)
{

}


bool checkLArg(LAttributP larg)
{

}


bool checkAff(char* id, TreeP expr, ClasseP lclasse, VarDeclP env)
{
    return (strcmp(id,checkExpr(expr,lclasse,env))==0);
}


bool checkListeInstr(TreeP tree, ClasseP lclasse, VarDeclP env)
{
    if(tree != NIL(Tree))
    {
        switch(tree->op)
        {
            case LINSTR :
                return (checkInstr(getChild(tree,0)) && checkListeInstr(getChild(tree,1)));
            default :
                return (checkInstr(tree,lclasse,env));
        }
    }
    return FALSE;
}

bool checkInstr(TreeP tree, ClasseP lclasse, VarDeclP env)
{
    if(tree->op !=  NIL(TreeP))
    {
        switch (tree->op)
        {
            case YEXPR :
                if(checkExpr(tree,lclasse,env)==NULL) 
                {
                    fprintf(stderr, "Erreur dans checkInstr YEXPR\n");
                    return FALSE;
                }
                return TRUE ;
            case EAFF :
                if(checkAff(getId(getChild(tree,0),env),tree,lclasse,env)==NULL) 
                {
                    fprintf(stderr, "Erreur dans checkInstr EAFF\n");
                    return FALSE;
                }
                return TRUE ;
            case YITE :
                if(checkExpr(getChild(tree,0),lclasse,env) != NULL && checkListeInstr(getChild(tree,1),lclasse,env) && checkListeInstr(getChild(tree,2),lclasse,env))
                {
                    return TRUE;
                }
                fprintf(stderr, "Erreur dans checkInstr YITE\n");
                return FALSE;
            case YCONT :
                if(TODO checkDeclChamp() && checkInstr(getChild(tree,1),lclasse,env))
                {
                    return TRUE;
                }
                fprintf(stderr, "Erreur dans checkInstr YCONT\n");
                return FALSE;
            case LINSTR :
                if(checkListeInstr(tree,lclasse,env))
                {
                    return TRUE;
                }
                fprintf(stderr, "Erreur dans checkInstr LINSTR\n");
                return FALSE;
            default :
                fprintf(stderr, "Erreur dans checkInstr\n");
                return FALSE;
        }
    }
    else
    {
        return TRUE;
    }
}

char *getId(TreeP tree, VarDeclP env)
{
    char *type = tree->u.str;
    if(checkPortee(env, type))
    {
        while(env != NIL(VarDecl))
        {
              if(strcmp(type, env->name) == 0)
            {
                return env->attr->type->nom;
            }
            env = env->next;
        }
    }
    return NULL;
}
*/