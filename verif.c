#include <string.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char*);
extern void setError(int code);

/*Verifie la portee d'une variable nom*/
/*bool checkPortee(VarDeclP lvar, char* nom)
{
	if(nom != NULL)
	{
    	while(lvar != NIL(VarDecl))
    	{
        	if(strcmp(lvar->name, nom)==0)
        	{
            	return TRUE;
        	}
        	lvar= lvar->next;
    	}
	}
    fprintf(stderr, "Erreur checkPortee pour la var : %s\n",nom);
    return FALSE;
}
*/
/*
void transmettreEnv(TreeP tree)
{
    int i;
    for(i=0; i<tree->nbChildren ; i++)
    {
        getChild(tree,i)->u.lvar = tree->u.lvar;    
    }
}
*/

/*Verifie si le nom est bien attribue a une classe defini*/
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
}
*/
/*Verifie qu'une expression*//*
char* checkExpr(TreeP tree, ClasseP classes, VarDeclP env)
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
            			{*/
            				/*return env->type->type->nom;*//*
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
                /* TODO *//*
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

/*
bool checkTypeAff(VarDeclP var, char* nom, ExpressionP expr)
{

}
bool checkLArg(LAttributP larg)
{

}
*/