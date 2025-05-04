#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "connection_protocol.h"
 
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

// verifica a soma de todos os bytes do buffer, fora o byte destinado a checksum
uchar local_checksum(uchar buffer[131]){
	uchar sum = 0;
	sum += buffer[0] + buffer[1] + buffer[2];
	for (int i=4; i<131; i++) sum += buffer[i];

	return sum;
}

// retorna 1 caso tenha sucesso
// retorna 0 caso parametros estejam fora dos limites
int local_build_message(uchar buffer[131], uchar size, uchar seq, uchar type, uchar data[127]){
	// verifica se parametros estão dentro dos limites
	if ((seq > 31) || (size > 127) || (type > 15)) return 0;

	// inicializa o buffer em 0 para não haver problemas
	for (int i=0; i<131; i++) buffer[i] = 0x00;

	// monta a mensagem
	buffer[0] = INIT_SEQUENCE;			   // marcador de inicio
	buffer[1] = (size << 1) | (seq & 0x01);	
	buffer[2] = (type << 4) | (seq >> 1);
	memcpy(&buffer[4], data, 127);			// copia os dados para o final do buffer
	buffer[3] = local_checksum(buffer);

	return 1;
}

// retorna 1 com sucesso
// retorna 0 caso contrario
int protocol_send_message(int socket, uchar size, uchar seq, uchar type, uchar data[127]){
	uchar buffer[131], aux[262];
	if(!local_build_message(buffer, size, seq, type, data)) return 0;

	// adiciona bytes 0xff depois de 0x88 e 0x81 e envia
	int j=0;
	for (int i=0; i<131; i++){
		aux[j] = buffer[i];
		if((buffer[i] == 0x88) || (buffer[i] == 0x81)){
			j++;
			aux[j] = 0xff;
		}
		j++;
	}

	if(send(socket, aux, 262, 0) == -1) return 0;
	return 1;
}

//
//======================================================================
//

void local_deconstruct_message(uchar buffer[131], uchar *size, uchar *seq, uchar *type, uchar data[127]){
	// desmonta a mensagem
	*size  = buffer[1] >> 1;
   *seq  = ((buffer[2] & 0x0F) << 1) | (buffer[1] & 0x01);
   *type = buffer[2] >> 4;
	memcpy(data, &buffer[4], 127);			// copia a area de dados do buffer para dados
}

// retorna 1 com sucesso
// retorna 0 caso contrario
int protocol_recieve_message(int socket, uchar buffer[131], uchar *size, uchar *seq, uchar *type, uchar data[127]){
	uchar aux[262];

	// recebe mensagem em aux
	if (recv(socket, aux, 262, 0) == -1) return 0;
	
	// remove os bytes 0xff depois de 0x88 e 0x81 e guarda no buffer
	int j = 0;
	for (int i=0; i<131; i++){
		buffer[i] = aux[j];
		if (((aux[j] == 0x88) || (aux[j] == 0x81)) && (aux[j+1] == 0xff)) j++;
		j++;
	}

	local_deconstruct_message(buffer, size, seq, type, data);

	// verifica checksum
	if (buffer[3] != local_checksum(buffer)) return 0;

	return 1;
}

