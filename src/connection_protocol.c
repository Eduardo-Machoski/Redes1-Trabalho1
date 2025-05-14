#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "connection_protocol.h"
 
// variavel global de controle de timeout
long long timeoutMillis = 1000;

int protocol_create_raw_socket(char* interface_name) {
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
 
    return new_socket;
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
	package.buffer[0] = INIT_SEQUENCE;				// marcador de inicio
	package.buffer[1] = (package.size << 1) | (package.seq >> 4);	
	package.buffer[2] = (package.seq << 4) | package.type;
	memcpy(&(package.buffer[4]), package.data, package.size);			// copia os dados para o final do buffer
	package.buffer[3] = local_checksum(package);
}

// retorna 1 com sucesso
// retorna 0 caso contrario
int protocol_send_package(int socket, package_t package){
	uchar buffer_size = 4 + package.size;		
	
	// verifica se parametros estão dentro dos limites
	if ((package.seq > 31) || (package.size > 127) || (package.type > 15)) return 0;

	// conta quantos bytes 0x88 e 0x81 tem na mensagem
	// só iram ocorrer esses bytes no campo data
	for (int i=0; i<package.size; i++)
		if ((package.data[i] == 0x81) || (package.data[i] == 0x88)) 
			buffer_size++;

	// aloca o pacote com base no seu tamanho total
	// pacotes precisam ter no minimo 14 bytes
	if (buffer_size < 14){
		buffer_size = 14;
		package.buffer = malloc(14);
		for (int i=0; i<14; i++) package.buffer[i] = 0x00;
	} else package.buffer = malloc(buffer_size);
	if (!package.buffer) exit(1);

	// constroi o pacote
	local_build_package(&package);
	
	uchar *aux = malloc(buffer_size);
	if (!aux) exit(1);

	// adiciona bytes 0xff depois de 0x88 e 0x81 e envia
	int j=0;
	for (int i=0; i<(4+package.size); i++){
		aux[j] = package.buffer[i];
		if((package.buffer[i] == 0x88) || (package.buffer[i] == 0x81)){
			j++;
			aux[j] = 0xff;
		}
		j++;
	}

	int status = send(socket, aux, buffer_size, 0); 

	free(buffer);
	free(aux);

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
	memcpy(data, &(package->buffer[4]), package->size);			// copia a area de dados do buffer para dados
}

// retorna 1 caso receba uma mensagem com marcador de inicio
// retorna 0 caso contrario
int protocol_recieve_package(int socket, package_t *package){ 
	uchar aux[262];

	// recebe pacote em aux
	if (!recv(socket, aux, 262, 0)) return 0;

	// verifica se tem marcador de inicio
	if (aux[0] != INIT_SEQUENCE) return 0;


	// remove os bytes 0xff depois de 0x88 e 0x81 e guarda no buffer
	int j = 0;
	for (int i=0; i<131; i++){
		package->buffer[i] = aux[j];
		if (((aux[j] == 0x88) || (aux[j] == 0x81)) && (aux[j+1] == 0xff)) j++;
		j++;
	}

	local_desconstruct_package(package);

	return 1;
}

// função de recieve sem timeout
// essa função serve para recieves passivos, que não são para respostas de sends
// retorna 0 caso a mensagem contenha erro
// retorna -1 caso a mensagem seja repetida
// retorna 1 caso nova mensagem
int recieve_passive(int socket, uchar expected_seq, package_t *package){
	// espera ate receber um pacote com mensagem de inicio
	while (!protocol_recieve_package(socket, package));

	if (package.buffer[3] != checksum(package)) return 0;
	if (package.seq != expected_seq) return -1;
	return 1;
}


// retorna 1 caso receba mensagem valida
// retorna 0 em caso de timeout
int recieve_active(int socket, package_t *current_package, package_t *last_package){ 
	// seta timeout
	long long comeco = timestamp();
	struct timeval timeout = { .tv_sec = timeoutMillis/1000, .tv_usec = (timeoutMillis%1000) * 1000 };
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
	uchar aux[262];

	do{	
		/*
		// recebe mensagem em aux
		recv(socket, aux, 262, 0);
		
		// remove os bytes 0xff depois de 0x88 e 0x81 e guarda no buffer
		int j = 0;
		for (int i=0; i<131; i++){
			buffer[i] = aux[j];
			if (((aux[j] == 0x88) || (aux[j] == 0x81)) && (aux[j+1] == 0xff)) j++;
			j++;
		}

		local_deconstruct_package(buffer, size, seq, type, data);

		// verifica checksum e INIT_SEQUENCE
		if ((buffer[3] == local_checksum(buffer, *size)) && (buffer[0] == INIT_SEQUENCE)){
			timeoutMillis = 1000;		//volta o timeout a 1 segundo	
			return 1;
		}
		*/
	}while(timestamp() - comeco <= timeoutMillis);
	
	// recuo exponencial
	timeoutMillis *= 2;
	return 0;
}

