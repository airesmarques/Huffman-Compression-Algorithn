#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>


# include "huffree.c"

/* Vari�veis Globais utilizadas */
FILE *fic_origem, *fic_destino;



typedef struct lista {
	unsigned int car;
	unsigned long int peso;
	struct lista *prox;
	NODO *e;
} NODO_LISTA, *nodo_lst_ptr;




/* Prot�tipos das funcoes */
void help(void);



nodo_lst_ptr cria_elemento(NODO *r)
{
 NODO_LISTA *p;
 if((p=(NODO_LISTA *)malloc(sizeof(NODO_LISTA)))==NULL)
  {
   printf("\nErro de alocamento de mem�ria");
   exit(MAU_ALOCAMENTO);
  }
 p->car=r->car;
 p->peso=r->peso;
 p->prox=NULL;
 p->e=r;


 return p;
}





nodo_lst_ptr insere_em_lista(NODO_LISTA *head, NODO *dados)
{
 NODO_LISTA *novo, *p;
 novo=cria_elemento(dados);
 p=head;
 while (p->prox!=NULL) p=p->prox;
 p->prox=novo;
 return p;
}




void nos_nivel_n(NODO *root, int n,NODO_LISTA *lista)
{
 if (root==NULL) return;          /* Arvore vazia :((                */
 if (n==1 ) { 			 /* Nivel desejado foi atingido :)) */
    if (lista==NULL) lista=cria_elemento(root);
     else insere_em_lista(lista,root);
    return;
  }

 nos_nivel_n(root->esq,n-1,lista);
 nos_nivel_n(root->dir,n-1,lista);
}




NODO_LISTA * cria_lista_nodos(NODO *root, int altura)
{
 NODO_LISTA *lista;
 int i;

 lista=cria_elemento(root);           /* Serve apenas para criar a lista */

 for (i=altura; i>=1; i--) {
    nos_nivel_n(root, i, lista);
   }

 return lista;
}




/* -----------------------------------------------------------------------
   |               Destroi completamente uma lista ligada                |
   -----------------------------------------------------------------------
    - Recursiva, avanca o ponteiro ate o ultimo elemento da lista.
    - Quando eh atingido o ultimo elemento, comeca a destruicao massiva
      dos elementos da lista.					          */
void destroi(nodo_lst_ptr head)

{
 if (head!=NULL) destroi(head->prox);
   free(head);
}



/* -----------------------------------------------------------------------
   |                                                                     |
   -----------------------------------------------------------------------*/
int detecta_trocas(NODO_LISTA *head)
{
 int valor_anterior;
 NODO_LISTA *backup, *actual;
 int troca=FALSE;
 NODO_LISTA *a, *b;   	       // Elementos a trocar, caso necessario

 actual=head;

 while (actual!=NULL && troca==FALSE) {
    valor_anterior=actual->peso;
    backup=actual;
    actual=actual->prox;
    if (actual->peso < valor_anterior) troca=TRUE;
  }

 if (!troca) return FALSE;

 a=backup;
 b=actual;
 while (b->peso==b->prox->peso) b=b->prox;

 valor_anterior=a->peso;                        /* Coloca o pont. "a" a       */
 a=head;                                       /* apontar para a primeira    */
 while (a->peso < valor_anterior) a=a->prox;  /* ocorrencia com o mesmo val */
 
 troca_ramos(a->e , b->e);
 return TRUE;
}




/* -----------------------------------------------------------------------
   |                                                                     |
   -----------------------------------------------------------------------*/
char * huffman_insere(NODO *root, unsigned int ch, int *altura)
{
 NODO *p;
 char *caminho;
 NODO_LISTA *head, *ptr_destruct;

 /* Se existe ent�o actualiza o peso do elemento existente */
 caminho=existe(root,ch,".");
 if (caminho!=NULL) caminho++;

 /* Se n�o existe, insere na posi��o e0 */
 if (caminho==NULL) {
  caminho=nao_existe(root, ch, ".");	
  strcat(caminho,uintchar_to_bin(ch));
  caminho++;
  (*altura)++;
 }

 /* Efectua as trocas necess�rias */

 head=cria_lista_nodos(root, *altura);

 ptr_destruct=head;
 free(ptr_destruct);

 head=head->prox;

 if (detecta_trocas(head)) {
  pesos(root);
  *altura=altura_arvore(root);
 }

 destroi(head);

 return caminho;
}




void huffman_actualiza_pesos(NODO *root, int altura)
{
 NODO_LISTA *head, *ptr_destruct;

 head=cria_lista_nodos(root, altura);

 ptr_destruct=head;
 free(ptr_destruct);

 head=head->prox;

 if (detecta_trocas(head)) pesos(root);
 destroi(head);
}




/* -----------------------------------------------------------------------
   |      Acrescenta o conteudo do buffer ao ficheiro destino.           |
   -----------------------------------------------------------------------
    - O conteudo do buffer encontra-se em bits.
    - Antes de colocar um caracter no ficheiro, tem que se converter os bits.
    - 8 bits equivalem a um caracter, por esse motivo o contador c_bits �
      incrementado � raz�o de 8.
    - Se houver um erro na escrita do ficheiro eh retornado o codigo -1.
    - Se tudo bem, a funcao retorna o numero de bytes escritos.
*/
int escreve_em_fic_encode(char *buffer, FILE *destino)
{
 int t_bits, n_bits, c_bits;       /* total de bits, bits escritos, conta bits */
 int n_bytes=0;  			/* Total de bytes escritos no ficheiro */
 unsigned int ch;

 t_bits=strlen(buffer);
 n_bits=(t_bits/8)*8;

 for(c_bits=0; c_bits<n_bits; c_bits+=8) {
   ch=fetch_char(buffer);
   buffer+=8;
   putc(ch, destino);
  }

 n_bytes=c_bits/8;
 if (n_bytes!=t_bits/8) return -1;
 return n_bytes;
}



/* -----------------------------------------------------------------------
   |         Faz a codificacao de huffman para um dado ficheiro.          |
   -----------------------------------------------------------------------
     - O ficheiro de origem eh apontado por origem
     - O ficheiro de destino eh apontado por destino
     - A arvore de huffman serah criada no NODO *root
     - output_string eh o output (em bits) para cada caracter lido do
       ficheiro de origem
     - buffer armazena todos os bits sob a forma de caracter
     - Para maior seguranca, o buffer eh "refrescado" sempre que estiver
       a menos de 255 bytes de atingir o seu limite.                      */
void huffman_procedure_encode(FILE *origem, FILE *destino)
{
 int BUFFER_SIZE=2048;
 int ch;
 NODO *root=NULL;
 char *output_string;                           /* String de bits  */
 char *buffer;                                 /* Armazem de bits */
 char *backup_buffer;
 int n_bytes;
 int altura=0;

 if ( ( buffer        = (char *)malloc(BUFFER_SIZE) ) == NULL ||
      ( backup_buffer = (char *)malloc(BUFFER_SIZE) ) == NULL    ) {
    printf("\nErro de alocamento na mem�ria");
    exit(MAU_ALOCAMENTO);
   }

 /* Inicializacao */
 if ((ch=getc(origem))!=EOF)
   root=cria_arvore(ch);
 strcpy(buffer, uintchar_to_bin(ch));              /* 1 Caracter do ficheiro */

 /* Processo de compressao */
 while ((ch=getc(origem))!=EOF) {
   output_string = huffman_insere(root, ch, &altura);
   strcat(buffer, output_string);

   if (strlen(buffer) >= BUFFER_SIZE-256) {
      n_bytes = escreve_em_fic_encode(buffer,destino);
      buffer += n_bytes * 8;     strcpy(backup_buffer,buffer);
      buffer -= n_bytes * 8;     strcpy(buffer,backup_buffer);
    }
  } /* Fim do ciclo while - Processo de compressao */

  /* Finalizacao */
  escreve_em_fic_encode(buffer,destino);
}


/* -----------------------------------------------------------------------
   |        Faz a descodificacao de huffman para um dado ficheiro.        |
   ----------------------------------------------------------------------- */
void huffman_procedure_decode(FILE *origem, FILE *destino)
{
 int BUFFER_SIZE=2048;
 int ch, deslocamento;
 char *ch_bin;
 NODO *root=NULL, *last;
 char *buffer, *backup_buffer;  			/* Buffers de bits */
 int decode_ch;
 int altura=0;

 if ( ( buffer        = (char *)malloc(BUFFER_SIZE) ) == NULL ||
      ( backup_buffer = (char *)malloc(BUFFER_SIZE) ) == NULL    ) {
    printf("\nErro de alocamento na mem�ria");
    exit(MAU_ALOCAMENTO);
   }

 /* Inicializa��o */
 if ((ch=getc(origem))!=EOF) {
    root=cria_arvore(ch);
    putc(ch, destino);
   }

 /* Proceso de descompressao */
 while ((ch=getc(origem))!=EOF) {
    ch_bin=uintchar_to_bin(ch);
    strcat(buffer,ch_bin);

    if (strlen(buffer) >= BUFFER_SIZE-256)
     while (strlen(buffer) > 512)
     {
       // Descodificar conteudo do buffer
       decode_ch=decode_char(buffer, root, &deslocamento, &altura);
       // Actualizar buffer
       buffer+=deslocamento;
       strcpy(backup_buffer, buffer);
       buffer-=deslocamento;
       strcpy(buffer, backup_buffer);

       // Escrever decode_ch em ficheiro
       putc(decode_ch, destino);

       // Actualizar a arvore
       huffman_actualiza_pesos(root, altura);
      }
  }
 //Descodificar caracteres restantes

 while (strlen(buffer) >= 1)
  {
   decode_ch=decode_char(buffer, root, &deslocamento, &altura);
   buffer+=deslocamento;             strcpy(backup_buffer, buffer);
   buffer-=deslocamento;             strcpy(buffer, backup_buffer);
   putc(decode_ch, destino);
   huffman_actualiza_pesos(root,altura);
  }
}




/* Fun��o main
   Parametros retornados: Retorna apenas o c�digo de erro (0=OK)
   Ac��es: Rotina principal
*/

int main(argc,argv)
	int argc;
	char *argv[];

{
 FILE *fic_origem, *fic_destino;

 if (argc!=4) {
	help();
	exit(MAUS_ARGUMENTOS); }

 else if (strcmp(argv[1],"-c")!=0 && strcmp(argv[1],"-d")!=0 ) {
	help();
	exit(MAUS_ARGUMENTOS); }

 else if ((fic_origem=fopen(argv[2],"rb"))==NULL) {
	help();
	exit(FICHEIRO_INVALIDO); }

 else if ((fic_destino=fopen(argv[3],"wb"))==NULL) {
	help();
	exit(FICHEIRO_INVALIDO); }

 else { /* HUFFMAN HERE */
	if (strcmp(argv[1],"-c")==0) huffman_procedure_encode(fic_origem, fic_destino);
	if (strcmp(argv[1],"-d")==0) huffman_procedure_decode(fic_origem, fic_destino);
  }

 fclose(fic_origem);
 fclose(fic_destino);
 return(NO_ERROR);
} /* fun��o main */











void help(void)
{
 printf("\nComprime um ficheiro utilizando o algoritmo de Huffman\n");
 printf("SINTAXE: darkhuff origem destino\n");
 printf("origem: Nome do ficheiro de origem\n");
 printf("destino: Nome do ficheiro de destino\n");
}
