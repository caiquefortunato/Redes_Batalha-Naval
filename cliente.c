#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
// Bibliotecas para socket
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <arpa/inet.h>
#include <dirent.h>
#include <netdb.h>

#define LEN 4096
#define TAM 10

struct sockaddr_in6 remoto;
struct hostent *server;
char mensagem[1] = {'C'}, atq[2];
int pontuacao = 0;

void ConverteIP(char *ip){ // Converte Ipv4 para IPv6
	char aux[100];
	strcpy(aux, "::FFFF:");
	strcat(aux,ip);
	strcpy(ip, aux);
}

// Inicializa a matriz que representa os oceanos
void inicializa_oceano(int oceano[TAM][TAM]){
    for(int i = 0; i < TAM; i++)
        for(int j = 0; j < TAM; j++)
            oceano[i][j] = 0;
}

// Imprime a matriz
void imprime_oceano(int oceano[TAM][TAM]){
    printf("---- Batalha Naval ----\n");
    printf("   0  1  2  3  4  5  6  7  8  9\n");
    char letra = '@';
    for(int i = 0; i < TAM; i++){
        printf("%c ", letra + 1);
        for(int j = 0; j < TAM; j++){
        	if(oceano[i][j] != -1) printf(" %d ", oceano[i][j]);
        	else printf("%d ", oceano[i][j]);
        }
        printf("\n");
        letra += 1;
    }
}

// Importa matriz com as posicoes definidas pelo cliente
void importa_posicao(int oceano[TAM][TAM]){
	char url[]="posicao.txt";
    char c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
	FILE *arq;
    int i = 0;

	arq = fopen(url, "r");
	if(arq == NULL)
        printf("Erro, nao foi possivel abrir o arquivo\n");
	else
		while((fscanf(arq,"%c %c %c %c %c %c %c %c %c %c\n",
                &c1,&c2,&c3,&c4,&c5,&c6,&c7,&c8,&c9,&c10))!=EOF){
                    oceano[i][0] = c1 - '0'; oceano[i][1] = c2 - '0'; 
                    oceano[i][2] = c3 - '0'; oceano[i][3] = c4 - '0';
                    oceano[i][4] = c5 - '0'; oceano[i][5] = c6 - '0';
                    oceano[i][6] = c7 - '0'; oceano[i][7] = c8 - '0';
                    oceano[i][8] = c9 - '0'; oceano[i][9] = c10 - '0';
                    i++;
                }	
	fclose(arq);
}

// Trata entrada do usuario para ataque
void escolhe_campo_ataque (int oceano_rival[TAM][TAM], int oceano[TAM][TAM]) {
    int linha, coluna;
    char escolha[2];

    while(1) {
        printf("Digite onde voce quer atacar\n");
        scanf("%s", escolha);

        if(escolha[0] == 'P'){
            printf("\n----- O que se sabe do campo do adversario -----\n");
            imprime_oceano(oceano_rival);
            printf("\n----- Meu campo (-1 e 1: onde recebi ataque) -----\n");
            imprime_oceano(oceano);
        }
            
        if(escolha[0] >= 'A' && escolha[0] <= 'J') {
            if(escolha[1] - '0' >= 0 && escolha[1] - '0' <= 9) {
                atq[0] = escolha[0];
                atq[1] = escolha[1];
               break;
            }
            else printf("Digite um valor entre 0 a 9\n\n");
        }
        else {
            if(escolha[0] != 'P') printf("Digite uma letra entre A a J\n");  
        }
    }
}

// Cria o buffer para envio do servidor
void envia_mensagem_servidor(char buffer[], int oceano_rival[TAM][TAM], int oceano[TAM][TAM]){
    escolhe_campo_ataque(oceano_rival, oceano);
    strcat(buffer, atq);
    strcat(buffer, mensagem);
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
    else if (flag == 'E'){
        oceano_rival[linha][coluna] = -1;
    }
    else if(flag == 'C'){}
    else printf("Erro no oceano rival\n");
}

// Traduz a mensagem enviada pelo servidor
void mensagem_servidor(char ataque[], int oceano[TAM][TAM], int oceano_rival[TAM][TAM]){
    int linha = 0, coluna = 0;
    char flag;

    for(int i = 0; i < 10; i++)
        if(ataque[0] == 'A' + i)
            linha = 0 + i;

    coluna = ataque[1] - '0';
    flag = ataque[2]; 

    if(flag == 'A') printf("Voce acertou um navio!\n");
    else if(flag == 'E') printf("Voce errou a ultima jogada!\n");
    else if(flag == 'C'){}
    else printf("Erro no ataque. Motivo: flag\n");
    printf("Voce foi atacado em: %c%d\n", ataque[0], coluna);

    oceano_rival_ataque(oceano_rival, flag);
    if(pontuacao != 30)
    	oceano_recebe_ataque(oceano, linha, coluna);

    printf("Sua pontuacao: %d\n\n", pontuacao);
}

int main(int argc, char *argv[]){
    int sockfd, len = sizeof(remoto), porta, inicio = 1; 
    char buffer[4096], ip[100];

    if(argc==3) {
        strcpy(ip, argv[1]);
        porta = atoi(argv[2]);

        // Cria socket
        sockfd = socket(AF_INET6, SOCK_STREAM, 0);
        if(sockfd < 0){ 
            perror("socket "); 
            exit(1);
        }

		server = gethostbyname2(ip,AF_INET6);
		if (server == NULL) {
			ConverteIP(ip);// Converte IPv4 para IPv6
			server = gethostbyname2(ip,AF_INET6);
			if(server == NULL){
				perror("host ");
				exit(1);
			}
		}

        // monta a estrutura de dados de endereço
        memset((char *) &remoto, 0x0, sizeof(remoto));
		remoto.sin6_flowinfo = 0;
		remoto.sin6_family = AF_INET6;
		memmove((char *) &remoto.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
		remoto.sin6_port = htons(porta); 

		//Faz a conexao com o servidor
        if(connect(sockfd,(struct sockaddr*)&remoto, len) == - 1){
            perror("connect ");
            exit(1);   
        }

        char ataque[2];
        int oceano[TAM][TAM];
        int oceano_rival[TAM][TAM];
        char mensagem_fim[1] = {'Z'};

        inicializa_oceano(oceano);
        inicializa_oceano(oceano_rival);
        importa_posicao(oceano);

        printf("Aguarde, posicionando navios...\n");
        while(1){
            if((recv(sockfd,buffer,LEN,0)) > 0) { // recebeu mensagem
                if(inicio == 1) { // Controle do inicio do jogo
                    printf("Jogo comecou!\n\n"); 
                    inicio = 0;
                }
                if(buffer[0] == 'Z') { // Se o servidor manda Z, o jogo terminou
                    printf("O servidor ganhou!\n");
                    break;
                }
                // Chama funcao para controlar mensagem do servidor
                mensagem_servidor(buffer, oceano, oceano_rival);
                if(pontuacao == 30) {  // Se eu ganhei envia mensagem e avisa o servidor
                    printf("Parabens! Voce ganhou do servidor!\n");
                    memset(buffer, 0x0, LEN);
                    strcat(buffer, mensagem_fim);
                    send(sockfd, buffer, strlen(buffer), 0);
                    break;
                }
            }
            memset(buffer, 0x0, LEN);
            envia_mensagem_servidor(buffer, oceano_rival, oceano);

            send(sockfd, buffer, strlen(buffer), 0);
        }
        
        close(sockfd);
        printf("Cliente encerrado!\n");
    } else printf("\nPadrao de entrada: IP + PORTA.\n");
    
    return 0;
}
