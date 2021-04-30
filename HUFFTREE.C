#include <stdio.h>  /* Para rotinas fgetc,fputc,fwrite e rewind */
#include <memory.h> /* Para rotinas memset,memcpy */
#include <malloc.h> /* Para rotinas malloc e free */
#include <stdlib.h> /* Para rotinas qsort e exit */
#include <string.h>

# include "huffbit.c"



/* C�digos de erro retornados para o sistema */
#define NO_ERROR 		0
#define FICHEIRO_INVALIDO 	1
#define MAUS_ARGUMENTOS		2
#define MAU_ALOCAMENTO		3
#define ERRO_ESCRITA_FICHEIRO   4


/* Constantes uteis */
#define	FALSE	0
#define	TRUE    1


typedef struct nodo {
	unsigned int car ;
	unsigned long int peso;
	struct nodo *esq, *dir;
} NODO, *p_arvore;




/* -----------------------------------------------------------------------
   |                       Prototipos de funcoes                         |
   ----------------------------------------------------------------------- */
int altura_arvore(NODO *root);
char * existe(NODO *p, unsigned int ch, char *v);
char * nao_existe(NODO *root, unsigned int ch, char *v);
void inorder(NODO *p);
NODO * cria_arvore(unsigned int ch);
int pesos(NODO *p);
NODO * novo_ramo(unsigned int ch);
void troca_ramos(NODO *a, NODO *b);
void recebe_elemento(char * str, p_arvore root);
int e_folha(p_arvore root);
int e_vazio(p_arvore root);
p_arvore encontra_elemento(char * str, p_arvore root, int *tamanho);
unsigned int decode_char(char *buffer, p_arvore root, int *deslocamento, int *altura);









/* -----------------------------------------------------------------------
   |      Retorna um inteiro com a altura de uma determinada arvore      |
   ----------------------------------------------------------------------- */
int altura_arvore(NODO *root)
{
 int altura,esq,dir;
  if (root==NULL) return 0;
  else {
    if ( (esq=altura_arvore(root->esq)) > (dir=altura_arvore(root->dir)) )
     altura=esq;
     else altura=dir;
     altura++;
   }
 return altura;
}





/* -----------------------------------------------------------------------
   |      Incrementa o peso de um elemento j� existente na arvore        |
   ----------------------------------------------------------------------- */
char * existe(NODO *p, unsigned int ch, char *v)
{
 char *esq, *dir;
 char  ve[512], vd[512];

 if (p==NULL) return NULL;

 if (p->car == ch) {
  p->peso++;
  return v;
 }

 strcpy(ve,v);
 strcat(ve,"0");
 if (p->esq!=NULL && (esq=existe(p->esq, ch, ve))!=NULL) {
   p->peso++;
   return esq;
  }

 strcpy(vd,v);
 strcat(vd,"1");
 if (p->dir!=NULL && (dir=existe(p->dir, ch, vd))!=NULL) {
   p->peso++;
   return dir;
  }

 return NULL;
}




/* -----------------------------------------------------------------------
   |                Adicciona um novo elemento ah arvore                 |
   ----------------------------------------------------------------------- */
char * nao_existe(NODO *root, unsigned int ch, char *v)
{
 NODO *pai, *filho;
 char *esq, *dir;
 char ve[512], vd[512];

 if (!root) return NULL;

 if (root->esq->car==300) { // a ovelha ranhosa eh o filho ah esquerda
   filho=novo_ramo(ch);
   pai=novo_ramo(500);

   pai->dir=filho;
   pai->esq=root->esq;

   root->esq=pai;   root->peso++;

   strcpy(ve,v);    strcat(ve,"0");
   return ve;
  }

 strcpy(ve,v);
 strcat(ve,"0");
 if (root->esq!=NULL && (esq=nao_existe(root->esq, ch, ve))!=NULL) {
   root->peso++;
   return esq;
  }

 strcpy(vd,v);
 strcat(vd,"1");
 if (root->dir!=NULL && (dir=nao_existe(root->dir, ch, vd))!=NULL) {
   root->peso++;
   return dir;
  }



 return NULL;
}






/* -----------------------------------------------------------------------
   |                Faz a listagem dos elemtentos da arvore              |
   ----------------------------------------------------------------------- */
void inorder(NODO *p)
{
 if(!p) return;
 inorder(p->esq);

 printf("\t");
 if (p->car==300) printf("e0");
  else if (p->car==500) printf("%d",p->peso);
   else {  putchar(p->car); printf("%d",p->peso);  }

 inorder(p->dir);
}




/* -----------------------------------------------------------------------
   |                         Cria uma nova arvore                        |
   ----------------------------------------------------------------------- */
/* - O elemento nulo, e0, tem peso 0 e o codigo utilizado para o reconhecer
     � 300.

   - A ra�z da arvore, tenha o peso que tiver, o codigo utilizado eh 500.

   - O codigo utilizado para uma raiz sem caracter, eh o mesmo da raiz da
     arvore, ou seja 500.
*/

NODO * cria_arvore(unsigned int ch)
{
 NODO *root;
 NODO *ramo_direito, *ramo_esquerdo;

 if((root=(NODO *)malloc(sizeof(NODO)))==NULL)
  {
   printf("\nErro de aloca��o na mem�ria");
   exit(MAU_ALOCAMENTO);
  }

 /* Cria��o do ramo direito */
 ramo_direito=novo_ramo(ch);

 /* Cria��o do ramo esquerdo */
  if((ramo_esquerdo=(NODO *)malloc(sizeof(NODO)))==NULL)
  {
   printf("\nErro de aloca��o na mem�ria");
   exit(MAU_ALOCAMENTO);
  }

 ramo_esquerdo->esq=NULL;
 ramo_esquerdo->dir=NULL;
 ramo_esquerdo->car=300;   /* O elemento de chave 300 e peso 0 */
 ramo_esquerdo->peso=0;    /* eh o elemento vazio              */

 root->esq=ramo_esquerdo;
 root->dir=ramo_direito;
 root->car=500;            /* Este elemento eh a raiz da arvore */
 root->peso=1;             /* A chave eh 500                    */

 return root;
}




/* -----------------------------------------------------------------------
   | CENA DOS PESOS
   ----------------------------------------------------------------------- */
int pesos(NODO *p)
{
 int peso_esquerdo, peso_direito;

	 if ((p->esq==NULL) && (p->dir==NULL)) return p->peso;

 peso_esquerdo=pesos(p->esq);
 peso_direito=pesos(p->dir);

 p->peso=peso_esquerdo+peso_direito;

 return p->peso;
}




/* -----------------------------------------------------------------------
   |                  Cria um novo ramo de uma arvore                    |
   ----------------------------------------------------------------------- */
NODO * novo_ramo(unsigned int ch)
{
 NODO *p;

 if((p=(NODO *)malloc(sizeof(NODO)))==NULL)
  {
   printf("\nErro de aloca��o na mem�ria");
   exit(MAU_ALOCAMENTO);
  }

 p->esq=NULL;
 p->dir=NULL;
 p->car=ch;
 p->peso=1;

 return p;
}





/* -----------------------------------------------------------------------
  |    Esta funcao troca o conteudo das aorvores apontadas por a e b      |
   ----------------------------------------------------------------------- */
void troca_ramos(NODO *a, NODO *b)
{
 NODO backup;

 backup = *a;
 *a=*b;
 *b=backup;
}




/* -----------------------------------------------------------------------
  | Para provar que um elemento eh folha numa arvore dinamica de Huffman  |
  | basta testar se o elemento apontado ah esquerda eh Nulo               |
   ----------------------------------------------------------------------- */
int e_folha(p_arvore root)
{
 if (root && root->esq==NULL) return TRUE;
 return FALSE;
}


/* -----------------------------------------------------------------------
  | Para provar que um elemento eh o elemento e0, basta ver se o seu      |
  | codigo eh o 300.                                                      |
   ----------------------------------------------------------------------- */
int e_vazio(p_arvore root)
{
 if (root->car == 300) return TRUE;
 return FALSE;
}


// Mudar para char *





/* -----------------------------------------------------------------------
  | Eh recebido um elemento que jah existe. Incrementa ocorrencia do el.  |
   -----------------------------------------------------------------------
   Repete
     Se o caracter for 0, vai para o ramo esquerdo
     Se o caracter for 1, vai para o ramo direito
   At� que seja encontrado um elemento folha                              */
p_arvore encontra_elemento(char * str, p_arvore root, int *tamanho)
{
 short int y,pos=0, PathSize=strlen(str);

 for(y=PathSize; y>=0; y--, pos++) {
   root->peso++;
   if (e_folha(root)) {/* Aqui deverah ser inserido o elemento novo */
     *tamanho=pos;     /* Aqui � retornado um valor inteiro         */
     return root; }    /* Aqui � retornado um ponteiro p_arvore     */

   if (str[pos]=='0') { /* Ramo ah esquerda */
     root=root->esq;
    }

   if (str[pos]=='1') { /* Ramo ah direita */
     root=root->dir;
    }
  }

 return NULL; /* Se retorna NULL, ent�o � porque n�o encontrou o que �
		 pedido,se isto acontecer � triste. O fic. t� danificado
		 ou entao nem sequer era um fic. compactado             */
}



/* -----------------------------------------------------------------------
  |          Descodifica o primeiro caracter existente no buffer.         |
   -----------------------------------------------------------------------
    - Avan�a o ponteiro at� encontrar uma folha
    - encontra_elemento() retorna sempre um elmento folha...
    - Caso essa folha seja o e0, ent�o pega nos 8 bits seguintes e transforma-os
      num caracter e acrescenta � arvore.
    - Retorna o numero de bits descodificados, esse numero de bits � dado
      pelo nivel em que a folha se encontra + 8 bits, caso se trate de e0.  */
unsigned int decode_char(char *buffer, p_arvore root,
			 int *deslocamento, int *altura)
{
 p_arvore p, filho_esq, filho_dir;
 NODO backup;
 unsigned int ch;
 int desl;         /* valor do numero de bits descodificados dentro da funcao */

 p=encontra_elemento(buffer, root, &desl);  /* alguns bits sao descodificados */

 if (e_vazio(p)) {
    buffer+=desl;                /* Avanca na string de bits */
    ch=fetch_char(buffer);       /* ch eh o novo caracter a inserir na arvore */
    desl+=8;                     /* Novo caracter -> + 8 bits descodificados  */

    filho_dir=novo_ramo(ch);     /* Ramo que ir� "acolher" o novo caracter */
    filho_esq=novo_ramo(300);    /* O "testemunho do elemento vazio  */
    filho_esq->peso=0;           /* Pertence agora a um novo ramo    */

    p->car=500;                  /* Aquele que era o elemento vazio passa */
    p->peso=1;                   /* agora a ser um elemento n�o folha     */

    p->esq=filho_esq;            /* aquele que era o e0 � agora o seu pai */
    p->dir=filho_dir;            /* ... e pai do novo caracter tambem     */
    (*altura)++;
  }
  else ch=p->car;
  /* N�o esquecer de ordenar a porra da arvore */
  *deslocamento=desl;            /* Numero total de bits descodificados   */
  return ch;                     /* Retorna o caracter descodificado      */
}



