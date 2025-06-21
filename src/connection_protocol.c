#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

#include "connection_protocol.h"
 

uchar g_expected_seq;     // Sequencia esperada da proxima mensagem a receber
uchar g_current_seq;      // Sequencia do pacote atual

int g_socket;				  // Variavel com o socket de comunicacao

package_t g_nack;         // Pacote nack
package_t g_last_package; // Ultimo pacote enviado por esse computador (atualizado todo envio)

void local_build_package(package_t *package);

// Envia novamente o ultimo pacote (exceto NACK) enviado pelo computador
void local_resend_package(){
	#ifdef DEBUG
	printf("local_resend_package\n");
	#endif
	send(g_socket, g_last_package.buffer, g_last_package.buffer_size, 0);
}

// Envia nack
void local_send_nack(){
	uchar buffer_size = 4 + g_nack.size;

	g_nack.seq = g_current_seq;

	// constroi o pacote
	local_build_package(&g_nack);

	// conta quantos bytes 0x88 e 0x81 tem na mensagem
	for (int i=0; i<g_nack.size + 4; i++)
		if ((g_nack.buffer[i] == 0x81) || (g_nack.buffer[i] == 0x88))
			buffer_size++;

	if (buffer_size < 14) buffer_size = 14;

	uchar *aux = malloc(buffer_size);
	if (!aux) exit(1);

	// adiciona bytes 0xff depois de 0x88 e 0x81 e envia
	int j=0;
	for (int i=0; i < (4 + g_nack.size); i++){
		aux[j] = g_nack.buffer[i];
		if((g_nack.buffer[i] == 0x88) || (g_nack.buffer[i] == 0x81)){
			j++;
			aux[j] = 0xff;
		}
		j++;
	}

	send(g_socket, aux, buffer_size, 0);
}



// Incrementa seq (vai de 0 a 31 circular)                            
void local_inc_seq(uchar *seq){                                                           
   *seq += 1;                                                            
                                                                                
   if(*seq == 32)                                                        
      *seq = 0;                                                          
}   

void local_create_raw_socket(char* interface_name) {
    // Cria arquivo para o socket sem qualquer protocolo
    int new_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (new_socket == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }
 
    int ifindex = if_nametoindex(interface_name);
 
    struct sockaddr_ll path = {0};
    path.sll_family = AF_PACKET;
    path.sll_protocol = htons(ETH_P_ALL);
    path.sll_ifindex = ifindex;
    // Inicializa socket
    if (bind(new_socket, (struct sockaddr*) &path, sizeof(path)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(-1);
    }
 
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    // Não joga fora o que identifica como lixo: Modo promíscuo
    if (setsockopt(new_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }
 
   g_socket = new_socket;
}

//
//======================================================================
//

void protocol_init(char* interface_name) {
	local_create_raw_socket(interface_name);

	// Inicializa valores de controle
	g_expected_seq = 0;
	g_current_seq = 0;

	// Inicializa o pacote NACK
	g_nack.size = 0;
	g_nack.type = NACK;
	g_nack.seq = 0;
}

//
//======================================================================
//

// retorna a soma dos bytes dos campos de sequencia, tipo, tamanho e dados
uchar local_checksum(package_t package){
	uchar sum = 0;
	sum += package.buffer[1] + package.buffer[2];
	for (int i=4; i<package.size; i++) sum += package.buffer[i];

	return sum;
}

// monta a mensagem
void local_build_package(package_t *package){
	package->buffer[0] = INIT_SEQUENCE;				// marcador de inicio
	package->buffer[1] = (package->size << 1) | (package->seq >> 4);	
	package->buffer[2] = (package->seq << 4) | package->type;
	memcpy(&(package->buffer[4]), package->data, package->size);			// copia os dados para o final do buffer
	package->buffer[3] = local_checksum(*package);
}

// retorna 1 com sucesso
// retorna 0 caso contrario
int protocol_send_package(package_t *package, bool inc){
	uchar buffer_size = 4 + package->size;		

	package->seq = g_current_seq;

	
	// verifica se parametros estão dentro dos limites
	if ((package->seq > 31) || (package->size > 127) || (package->type > 15)) return 0;
	
	// constroi o pacote
	local_build_package(package);

	// conta quantos bytes 0x88 e 0x81 tem na mensagem
	for (int i=0; i<package->size + 4; i++)
		if ((package->buffer[i] == 0x81) || (package->buffer[i] == 0x88)) 
			buffer_size++;

	if (buffer_size < 14) buffer_size = 14;

	uchar *aux = malloc(buffer_size);
	if (!aux) exit(1);

	// adiciona bytes 0xff depois de 0x88 e 0x81 e envia
	int j=0;
	for (int i=0; i<(4+package->size); i++){
		aux[j] = package->buffer[i];
		if((package->buffer[i] == 0x88) || (package->buffer[i] == 0x81)){
			j++;
			aux[j] = 0xff;
		}
		j++;
	}

	// Salva o pacote como o ultimo pacote enviado
	g_last_package.type = package->type;
	g_last_package.size = package->size;
	g_last_package.seq  = package->seq;
	g_last_package.buffer_size = buffer_size;
	for(int i = 0; i < package->size; i++)
		g_last_package.data[i] = package->data[i];
	for(int i = 0; i < buffer_size; i++)
		g_last_package.buffer[i] = aux[i];

	int status = send(g_socket, aux, buffer_size, 0); 

	free(aux);

	#ifdef DEBUG
		printf("Enviado seq: %u, Tipo: %u\n", g_current_seq, package->type);
	#endif

	if (inc) local_inc_seq(&g_current_seq);
	if (status == -1) return 0;
	return 1;
}

//
//======================================================================
//

// usando long long pra (tentar) sobreviver ao ano 2038
long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}

void local_deconstruct_package(package_t *package){
	// desmonta a mensagem
	package->size  = package->buffer[1] >> 1;
   	package->seq  = ((package->buffer[1] & 0b1) << 4) | (package->buffer[2] >> 4);
   	package->type = package->buffer[2] & 0b1111;
	memcpy(package->data, &(package->buffer[4]), package->size);			// copia a area de dados do buffer para dados
}

// retorna 1 caso receba uma mensagem com marcador de inicio
// retorna 0 caso contrario
int local_recieve_package(package_t *package){ 
	uchar aux[262];

	// recebe pacote em aux
	if (!recv(g_socket, aux, 262, 0)) return 0;

	// verifica se tem marcador de inicio
	if (aux[0] != INIT_SEQUENCE) return 0;

	// remove os bytes 0xff depois de 0x88 e 0x81 e guarda no buffer
	int j = 0;
	for (int i=0; i<131; i++){
		package->buffer[i] = aux[j];
		if (((aux[j] == 0x88) || (aux[j] == 0x81)) && (aux[j+1] == 0xff)) j++;
		j++;
	}

	local_deconstruct_package(package);

	return 1;
}

// função de recieve sem timeout
// essa função serve para recieves passivos, que não são para respostas de sends
// Verifica:
	// Checksum
	// Sequencia da mensagem
void protocol_recieve_passive(package_t *package){

	bool valid = false;

	//Repete ate receber uma resposta valida e com sequencia correta
	while(!valid){
		// Recebe um pacote com mensagem de inicio
		while (!local_recieve_package(package));

		// Verifica o checksum do pacote
		if (package->buffer[3] != local_checksum(*package)){
			// Erro checksum -> Envia nack e volta a esperar a mensagem
			local_send_nack();
			continue;
		}

		// Sucesso checksum -> Verifica a sequencia da mensagem recebida

		#ifdef DEBUG
			printf("Recebido seq: %u : esperado: %u, Tipo: %u\n",package->seq, g_expected_seq, package->type);
		#endif

      if(package->seq < g_expected_seq){   	  // Sequencia menor que esperado -> Envia a ultima mensagem enviada novamente
			local_resend_package();
      	continue;
		}else if (package->seq > g_expected_seq) // Sequencia maior que esperado -> Envia mensagem de erro sequencia -> Encerra o programa
         printf("Tem que fazer");

		// Se recebeu um nack envia a ultima mensagem novamente e continua no loop
		if(package->type == NACK){
			local_resend_package();
			continue;
		}

		// Obteve mensagem valida
		valid = true;
      // Atualiza a sequencia de mensagem esperada
      local_inc_seq(&g_expected_seq);
	}
}

// função de recieve com timeout
// essa função serve para recieves ativos, que são de respostas de sends
// Verifica:
	// Checksum
	// Sequencia
// Contem timeout
void protocol_recieve_active(package_t *package){

	bool valid = false;

	long long beggining;
	struct timeval timeout;
	long long timeoutMillis = 1000;

	while(!valid){
		beggining = timestamp();
		timeout.tv_sec = timeoutMillis/1000;
		timeout.tv_usec = (timeoutMillis%1000) * 1000;
   		setsockopt(g_socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));

		do
			if(local_recieve_package(package)){	
				timeoutMillis = 1000;			// volta o tempo de timeout original

				// Verifica o checksum do pacote
				if (package->buffer[3] != local_checksum(*package)){
					// Erro checksum -> Envia nack e volta a esperar a mensagem
					local_send_nack();
					continue;
				}

				// Sucesso checksum -> Verifica a sequencia da mensagem recebida

				#ifdef DEBUG
					printf("Recebido seq: %u : esperado: %u, Tipo: %u\n",package->seq, g_expected_seq, package->type);
				#endif

				if(package->seq < g_expected_seq){  		// Sequencia menor que esperado -> Envia a ultima mensagem enviada novamente
					local_resend_package();
					continue;
				} else if (package->seq > g_expected_seq)	// Sequencia maior que esperado -> Envia mensagem de erro sequencia -> Encerra o programa
					printf("Tem que fazer");


				switch (package->type){
					case NACK:
						// Se recebeu um nack envia a ultima mensagem novamente e continua no loop
						local_resend_package();
						continue;
					break;
					
					// Tipos validos para resposta de mensagens
					case ACK:
					case OK:
					case VIDEO:
					case IMAGEM:
					case TEXTO:
						valid = true;

						// Atualiza a sequencia de mensagem esperada
						local_inc_seq(&g_expected_seq);
					break;

					default:
						// Fazer envio de erro
				}


			}
		while(!valid && (timestamp() - beggining <= timeoutMillis));

		// Timeout estourado
		if(!valid)
			local_resend_package();

		// recuo exponencial do tempo de timeout
		timeoutMillis = timeoutMillis << 1;
	}

}

