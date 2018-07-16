#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
// Bibliotecas para socket
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <arpa/inet.h>
#include <dirent.h>
#include <netdb.h>

#define LEN 4096 // Tamanho da mensagem a ser lida
#define TAM 10

char mensagem[1] = {'C'}, ult_ac, atq[2];
int pontuacao = 0;
struct sockaddr_in6 local, remoto;

void traduz_linha_coluna(int *l, int *c){
    for(int i = 0; i < 10; i++)
        if(atq[0] == 'A'+i) *l = 0 + i;
    *c = atq[1] - '0';
}

void traduz_linha(int *l, char *l1){
    for(int i = 0; i < 10; i++)
        if(*l == 0) *l1 = 'A' + i;
}

void seleciona_aleatorio(int oceano_rival[TAM][TAM]) {
    int l, c;
    while(1){
        l = rand() % 10;
        c = rand() % 10;
        if(oceano_rival[l][c] == 0) break; 
    }
    
    for(int i = 0; i < 10; i++)
        if (l == i) atq[0] = 'A' + i;

    for(int i = 0; i < 10; i++)
        if (c == i) atq[1] = '0' + i;
}

void escolhe_campo_ataque(int oceano_rival[TAM][TAM]) {
    int l1, c1, h1;
    char buf[1], aux;

    if(ult_ac == 'A') {
        traduz_linha_coluna(&l1, &c1);
        if(c1 + 1 <= 9 && oceano_rival[l1][c1+1] != 1) {  // Tem como ir para direita
            sprintf(buf,"%i",c1+1);
            atq[1] = buf[0];           
        } else if (c1 - 1 >= 0 && oceano_rival[l1][c1-1] != 1){ // esquerda
            sprintf(buf,"%i",c1-1);
            atq[1] = buf[0];   
        } else if (l1-1>=0 && oceano_rival[l1-1][c1] != 1) { // baixo
            h1 = c1 - 1;
            traduz_linha(&h1, &aux);
            atq[0] = aux;
        } else if (l1+1<=9 && oceano_rival[l1+1][c1] != 1) { // cima
            h1 = c1 + 1;
            traduz_linha(&h1, &aux);
            atq[0] = aux;
        } else seleciona_aleatorio(oceano_rival);
    } else seleciona_aleatorio(oceano_rival);

}

void envia_mensagem(char buffer[], int oceano_rival[TAM][TAM]) {
    escolhe_campo_ataque(oceano_rival);
    strcat(buffer, atq);
    strcat(buffer, mensagem);
    //printf("Enviado: %s\n", buffer);
}

// Constroi matriz inicial para jogo
void inicializa_oceano(int oceano[TAM][TAM]){
    for(int i = 0; i < TAM; i++)
        for(int j = 0; j < TAM; j++)
            oceano[i][j] = 0;
}

// Cria um posicionamento randomico
int posiciona_navio(int t, int oceano[TAM][TAM]){
    int c = 0, l = 0, d = 0;
    srand(time(NULL));
    d = rand() % 2;

    if(d == 0){ // se quero na horizontal
        c = rand() % (10 - t);
        l = rand() % 10;

       for(int i = c; i < t + c; i++)
            if(oceano[l][i] != 0)
                return 1;

        for(int i = c; i < t + c; i++)
            oceano[l][i] = t;
    } else { // se quero na vertical
        c = rand() % 10;
        l = rand() % (10 - t);

        for(int i = l; i < t + l; i++)
            if(oceano[i][c] != 0)
                return 1;

        for(int i = l; i < t + l; i++)
            oceano[i][c] = t;
    }
    return 0;
}

// Chama funcao para criar as posicoes aleatorias
void cria_posicoes(int oceano[TAM][TAM]) {
    printf("Aguarde... Posicionando o porta aviao\n");
    while(posiciona_navio(5, oceano));
    printf("Aguarde... Posicionando navios tanque\n");
    for(int i = 0; i < 2; i++) while(posiciona_navio(4, oceano));
    printf("Aguarde... Posicionando contra torpedeiros\n");
    for(int i = 0; i < 3; i++) while(posiciona_navio(3, oceano));
    printf("Aguarde... Posicionando submarinos\n");
    for(int i = 0; i < 4; i++) while(posiciona_navio(2, oceano));
}

// Atualiza a minha matriz de oceano com o ataque
void oceano_recebe_ataque(int oceano[TAM][TAM], int linha, int coluna){
    int posicao_atacada = oceano[linha][coluna];

    if(posicao_atacada == 0){ // Se for 0 nao atacou e errou: -1
        strcpy(mensagem, "E");
        oceano[linha][coluna] = -1;        
    }
    else if(posicao_atacada == -1) // Se for -1: atacou e errou
        strcpy(mensagem, "E");
    else if(posicao_atacada == 1) // Se for 1: atacou e acertou
        strcpy(mensagem, "E");
    else{ // Se for ! de 0, acertou: 1
        strcpy(mensagem, "A");
        oceano[linha][coluna] = 1;
    }
}

// Atualiza a minha matriz de oceano com o ataque
void oceano_rival_ataque(int oceano_rival[TAM][TAM], char flag){
    int linha = 0, coluna = 0;
    if(atq[0] == 'A') linha = 0; if(atq[0] == 'B') linha = 1;
    if(atq[0] == 'C') linha = 2; if(atq[0] == 'D') linha = 3;
    if(atq[0] == 'E') linha = 4; if(atq[0] == 'F') linha = 5;
    if(atq[0] == 'G') linha = 6; if(atq[0] == 'H') linha = 7;
    if(atq[0] == 'I') linha = 8; if(atq[0] == 'J') linha = 9;
    coluna = atq[1] - '0';

    if(flag == 'A') {
        oceano_rival[linha][coluna] = 1;
        pontuacao += 1;
    }
    else if (flag == 'E') oceano_rival[linha][coluna] = -1;
    else if (flag == 'C') {}
    else printf("Erro no ataque\n");
}

// Traduz a mensagem enviada pelo cliente
void mensagem_cliente(char ataque[], int oceano[TAM][TAM], int oceano_rival[TAM][TAM]){
    int linha = 0, coluna = 0;
    char flag = 0;
    //printf("Recebido: %s\n", ataque);
    if(ataque[0] == 'A') linha = 0; if(ataque[0] == 'B') linha = 1;
    if(ataque[0] == 'C') linha = 2; if(ataque[0] == 'D') linha = 3;
    if(ataque[0] == 'E') linha = 4; if(ataque[0] == 'F') linha = 5;
    if(ataque[0] == 'G') linha = 6; if(ataque[0] == 'H') linha = 7;
    if(ataque[0] == 'I') linha = 8; if(ataque[0] == 'J') linha = 9;
    coluna = ataque[1] - '0';
    flag = ataque[2];
    ult_ac = ataque[2];

    oceano_recebe_ataque(oceano, linha, coluna);
    oceano_rival_ataque(oceano_rival, flag);
}

int main(int argc, char *argv[]){
    int sockfd, cliente, len = sizeof(remoto), porta;
    char buffer[4096];

    if(argc == 2) {
        porta = atoi(argv[1]);
        sockfd = socket(AF_INET6, SOCK_STREAM, 0);
        if(sockfd == -1){
            perror("socket "); 
            exit(1);
        } 

        bzero((char *) &local, sizeof(local));
        local.sin6_flowinfo = 0;
        local.sin6_family = AF_INET6;
        local.sin6_addr = in6addr_any;
        local.sin6_port = htons(porta);

        if(bind(sockfd,(struct sockaddr*)&local, sizeof(local))==-1){
            perror("bind ");
            exit(1);
        }

        listen(sockfd, 1);
        if((cliente = accept(sockfd,(struct sockaddr*)&remoto, &len)) == - 1){
            perror("accept ");
            exit(1);   
        }

        char ataque[2];
        int oceano[TAM][TAM];
        int oceano_rival[TAM][TAM];
        int flag = 0;
        char mensagem_fim[1] = {'Z'};

        inicializa_oceano(oceano);
        inicializa_oceano(oceano_rival);
        cria_posicoes(oceano);

        while(flag == 0){
            envia_mensagem(buffer, oceano_rival);

            if(send(cliente, buffer, strlen(buffer), 0)){
                while(1){
                    if((recv(cliente,buffer,LEN,0)) > 0) { // recebeu mensagem
                        if(buffer[0] == 'Z') {
                            printf("O cliente ganhou!\n");
                            close(cliente); 
                            flag = 1;
                            break;
                        }
                        
                        if(pontuacao == 30) {
                            memset(buffer, 0x0, LEN);
                            strcat(buffer, mensagem_fim);
                            send(cliente, buffer, strlen(buffer), 0);				    	    	
                            flag = 1;
                            break;                   	
                        }
                        
                        mensagem_cliente(buffer, oceano, oceano_rival);
                        memset(buffer, 0x0, LEN);
                        break;
                    }
                }
            }
        }

        close(cliente);
        close(sockfd);
        printf("Servidor encerrado!\n");
    }
    else printf("\nPadrao de entrada: PORTA.\n");

    return 0;
}