/****************************************************************************
 *
 * PROJECT:             Wireless Sensor Network for Green Building
 *
 * AUTHOR:              Debby Nirwan
 *
 * DESCRIPTION:         DBP Server Implementation
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2014. All rights reserved
 *
 ***************************************************************************/

/* standard include */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <limits.h>

/* local include */
#include "queue.h"
#include "newLog.h"
#include "newDb.h"
#include "json.h"
#include "jsonCreate.h"
#include "gateway.h"
#include "parsing.h"
#include "dbp.h"
#include "dbp_search.h"
#include "dbp_loc_receiver.h"

/* version */
#define DBP_MAJOR_VERSION    "0"
#define DBP_MINOR_VERSION    ".9"

/* defines */
#define DBP_PORT     "2002"
#define INPUTBUFFERLEN  MAXMESSAGESIZE
#define MAX_NUM_OF_CONN    100

/* struct */
typedef struct{
    bool connected;
    uint8_t conn_number;
    int sock_fd[MAX_NUM_OF_CONN];
}connection_t;

/* global variable */
pthread_t zb_msg_thread, dbp_msg_thread;
pthread_mutex_t dbp_mutex = PTHREAD_MUTEX_INITIALIZER;
int dbpQueue = -1;
char inputBuffer[INPUTBUFFERLEN+2];
connection_t connection;
char version[13] = {0};

#define NUMINTATTRS  9
char * intAttrs[NUMINTATTRS] = {"tmp", "hum", "als", "lqi", "bat", "batl", "xloc", "yloc", "zloc"};

#define NUMSTRINGATTRS  3
char * stringAttrs[NUMSTRINGATTRS] = {"mac", "nm", "par"};
int    stringMaxlens[NUMSTRINGATTRS] = {16, 16, 16};

void dbp_external_send_data_to_clients(char *data);

/* local function prototypes */
static void *zigbee_msg_receiver(void *arg);
static void *dbp_client_handler(void *arg);
static void dbp_msg_receiver(int socket_fd);
static void dbp_init(void);
static int dbp_update_battery(char *mac);
static int dbp_update_battery_lvl(char *mac);
static int dbp_update_lqi(char *mac);
static int dbp_update_location(char *mac);
static int dbp_update_parent_address(char *mac);
static int dbp_update_illuminance(char *mac);
static int dbp_update_humidity(char *mac);
static int dbp_update_temperature(char *mac);
static int dbp_get_index_number(void);
static void dbp_add_socket(int sock_fd, int index);
static void dbp_remove_socket(int index);
static int dbp_get_socket(int index);
static int dbp_send_data_to_clients(char *data);
static void dbp_location_info(char *mac);

static void dbp_onError(int error, char * errtext, char * lastchars) {
    printf("onError( %d, %s ) @ %s\n", error, errtext, lastchars);
}

static void dbp_onObjectStart(char * name) {
    printf("onObjectStart( %s )\n", name);
    parsingReset();
}

static void dbp_onObjectComplete(char * name) {
    
    char *mac = NULL;
    bool status = false;
    
    printf("onObjectComplete\n");
    
    pthread_mutex_lock(&dbp_mutex);
    status = connection.connected;
    pthread_mutex_unlock(&dbp_mutex);
    
    if(strcmp(name, "sensor") == 0){
        
        /* check location info first */
        mac = parsingGetStringAttr("mac");
        
        if( (isEmptyString(parsingGetStringAttr("nm")) == 0) ||
			(parsingGetIntAttr("xloc") != INT_MIN) ||
			(parsingGetIntAttr("yloc") != INT_MIN) ||
			(parsingGetIntAttr("zloc") != INT_MIN) ){
            
            printf("%s: location information received\n", __func__);
            
            /* location information */
            dbp_location_info(mac);
            
        }else{
            
            if(status == true){
                
                printf("%s: ok. status = %d\n", __func__, status);
                
                /* dummy calls */
                dbp_update_parent_address(mac);
                dbp_update_location(mac); 
                dbp_update_lqi(mac);
                
                dbp_update_battery(mac);
                if(dbp_update_battery_lvl(mac) == 0) return;
                if(dbp_update_illuminance(mac) == 0) return;
                if(dbp_update_humidity(mac) == 0) return;
                if(dbp_update_temperature(mac) == 0) return;
                
            }else{
                printf("%s: error. status = %d\n", __func__, status);
            }
            
        }
        
    }else{
        printf("%s: error. name = %s\n", __func__, name);
    }
    
}

static void dbp_onArrayStart(char * name) {
    printf("onArrayStart( %s )\n", name);
}

static void dbp_onArrayComplete(char * name) {
    printf("onArrayComplete( %s )\n", name);
}

static void dbp_onString(char * name, char * value) {
    printf("onString( %s, %s )\n", name, value);
    parsingStringAttr( name, value );
}

static void dbp_onInteger(char * name, int value) {
    printf("onInteger( %s, %d )\n", name, value);
    parsingIntAttr( name, value );
}

static void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static void dbp_location_info(char *mac)
{
    char send_buf[512] = {0};
    
    if(isEmptyString(mac) == 0){
        
        if(isEmptyString(parsingGetStringAttr("nm")) == 0){
            if(dbp_report_location(parsingGetStringAttr("mac"), send_buf) == 0){
				/* send to clients */
                dbp_send_data_to_clients(send_buf);
            }
        }
        
    }else{
        printf("location information is empty");
    }
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    /* version number */
    sprintf(version, "Version %s%s", DBP_MAJOR_VERSION, DBP_MINOR_VERSION);
    
    newLogAdd( NEWLOG_FROM_DBP, "DBP started");
    newLogAdd( NEWLOG_FROM_DBP, version);
    
    /* init dbp */
    dbp_init();
    
    /* init dbp search */
    dbp_search_init();
    
    /* init dbp location receiver */
    dbp_loc_receiver_init();
    
    /* create thread for message receiver */
    if(pthread_create(&zb_msg_thread, NULL, &zigbee_msg_receiver, NULL) != 0){
        perror("Failed to create zigbee message receiver thread");
        return 1;
    }
        
    /* setup socket */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if( (rv = getaddrinfo(NULL, DBP_PORT, &hints, &servinfo)) != 0 ) {
        printf("getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        
        if( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ) {
            perror("server: socket");
            continue;
        }
        
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        /* disable Nagle's Algorithm to allow direct send  */
        if(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int)) == -1) {
            perror("TCP_NODELAY");
        }
        
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    if(p == NULL) {
        newLogAdd( NEWLOG_FROM_DBP,"server: failed to bind");
        return 2;
    }
    
    freeaddrinfo(servinfo);
    
    if(listen(sockfd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    
    while(1){
        
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if(new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr.ss_family, get_in_addr( (struct sockaddr *) &their_addr ), s, sizeof s);
        printf("server: got connection from %s\n", s);
        
        dbp_msg_receiver(new_fd);
        
    }
    
    return 0;
}

static void *zigbee_msg_receiver(void *arg)
{
    int i;
#if 0
    int cnt = 0;
#endif
    
    /* receive message */
    printf("Message Receiver's Thread is running\n");
    
    if( (dbpQueue = queueOpen(QUEUE_KEY_DBP, 0)) != -1 ){
    
        jsonSetOnError(dbp_onError);
        jsonSetOnObjectStart(dbp_onObjectStart);
        jsonSetOnObjectComplete(dbp_onObjectComplete);
        jsonSetOnArrayStart(dbp_onArrayStart);
        jsonSetOnArrayComplete(dbp_onArrayComplete);
        jsonSetOnString(dbp_onString);
        jsonSetOnInteger(dbp_onInteger);
        jsonReset();
        
        int numBytes = 0;
        
        while(1){
        
            numBytes = queueRead(dbpQueue, inputBuffer, INPUTBUFFERLEN);
            printf("Message received. numBytes = %d\n", numBytes);
            
            jsonReset();
            
            for(i=0; i<numBytes; i++){
                jsonEat(inputBuffer[i]);
            }
            
#if 0
            if(cnt++ > 10){
                cnt = 0;
                checkOpenFiles(4);
            }
#endif
        
        }
        
        queueClose(dbpQueue);
        
    }else{
        newLogAdd( NEWLOG_FROM_DBP,"Could not open dbp-queue");
    }
    
    return NULL;
}

static void *dbp_client_handler(void *arg)
{
    int byte_count;
    char buf[512];
    char output[512];
    int bytes_sent;
    int index = *((int *) arg);
    int socket_fd = dbp_get_socket(index);
    char *node_cmd = NULL;
    
    memset(buf, 0, sizeof(buf));
    memset(output, 0, sizeof(output));
    
    /* receive data from client here */
    while(1){
    
        byte_count = recv(socket_fd, buf, sizeof buf, 0);
        
        if(byte_count == 0){
            newLogAdd( NEWLOG_FROM_DBP,"Connection has been closed");
            dbp_remove_socket(index);
            break;
        }else if(byte_count < 0){
            /* something wrong has happened */
            newLogAdd( NEWLOG_FROM_DBP,"Connection error");
            dbp_remove_socket(index);
            break;
        }else{
            printf("Message received from client\n");
            /* pass it to dpb parser */
            dbp_parse_data(buf, sizeof(buf), output, 512, &node_cmd);
            /* if it is close link command stop here */
            if(strcmp(output, "dbp close") == 0){
                memset(buf, 0, sizeof(buf));
                memset(output, 0, sizeof(output));
                close(socket_fd);
                dbp_remove_socket(index);
                break;
            }else if(strcmp(output, "dbp node") == 0){
                if(node_cmd != NULL){
                    bytes_sent = send(socket_fd, node_cmd, strlen(node_cmd), 0);
                }
            }else{
                bytes_sent = send(socket_fd, output, strlen(output), 0);
                memset(buf, 0, sizeof(buf));
                memset(output, 0, sizeof(output));
            }
        }
        
    }
    
    pthread_exit(NULL);
    
    return NULL;
}

static void dbp_msg_receiver(int socket_fd)
{
    pthread_t client_handler_thread;
    int index = dbp_get_index_number();
    
    if(index < 0){
        printf("%s: index = %d. error, reached max number of connection\n", __func__, index);
        return;
    }
    
    dbp_add_socket(socket_fd, index);
    
    if(pthread_create(&client_handler_thread, NULL, &dbp_client_handler, (void *) &index) != 0){
        perror("Failed to create zigbee message receiver thread");
        return;
    }
    
    pthread_detach(client_handler_thread);
    
    usleep(100*1000);
}
 
static void dbp_init(void)
{
    int i;
    
    for(i=0; i<NUMINTATTRS; i++){
        parsingAddIntAttr(intAttrs[i]);
    }
    
    for(i=0; i<NUMSTRINGATTRS; i++) {
        parsingAddStringAttr(stringAttrs[i], stringMaxlens[i]);
    }
    
    connection.connected = false;
    connection.conn_number = 0;
    memset(connection.sock_fd, 0, sizeof(connection.sock_fd));
}

static int dbp_update_battery(char *mac)
{
    int value;
    char output[100];
//    char output_capa[100];
    
    value = parsingGetIntAttr("bat");
    printf("%s: value = %d\n", __func__, value);
    if(value >= 0){
        if(dbp_report_bat((uint16_t) value, mac, output) == 0){
            dbp_send_data_to_clients(output);
            printf("bat voltage sent to client");
        }
    }else{
        printf("bat parsing failed");
        return -1;
    }
    
    return 0;
}

static int dbp_update_battery_lvl(char *mac)
{
    int value;
    char output[100];
    
    value = parsingGetIntAttr("batl");
    printf("%s: value = %d\n", __func__, value);
    if(value >= 0){
        if(dbp_report_bat_lvl((uint16_t) value, mac, output) == 0){
            dbp_send_data_to_clients(output);
            printf("bat level sent to client");
        }
    }else{
        printf("bat parsing failed");
        return -1;
    }
    
    return 0;
}

static int dbp_update_lqi(char *mac)
{
    int value;
    
    value = parsingGetIntAttr("lqi");
    if(value >= 0){
        //printf("lqi sent to client");
    }else{
        printf("lqi parsing failed");
        return -1;
    }
    
    return 0;
}

static int dbp_update_location(char *mac)
{
    char *loc = NULL;
    
    loc = parsingGetStringAttr("loc");
    if(loc != NULL){
        //if(dbp_report_location(0, 0, 0,  loc_id, mac, output) == 0){
        //    bytes_sent = send(connection.sock_fd, output, strlen(output), 0);
        //    printf("loc sent to client");
        //}
    }else{
        printf("location parsing failed");
        return -1;
    }
    
    return 0;
}

static int dbp_update_parent_address(char *mac)
{
    char *par = NULL;
    
    par = parsingGetStringAttr("par");
    if(par != NULL){
        //printf("par sent to client");
    }else{
        printf("parent parsing failed");
        return -1;
    }
    
    return 0;
}

static int dbp_update_illuminance(char *mac)
{
    int value;
    char output[100];
    
    value = parsingGetIntAttr("als");
    if(value >= 0){
        if(dbp_report_als((uint16_t) value, mac, output) == 0){
            dbp_send_data_to_clients(output);
            printf("als sent to client");
        }
    }else{
        printf("als parsing failed");
        return -1;
    }
    
    return 0;
}

static int dbp_update_humidity(char *mac)
{
    int value;
    char output[100];
    
    value = parsingGetIntAttr("hum");
    if(value >= 0){
        if(dbp_report_rh((uint16_t) value, mac, output) == 0){
            dbp_send_data_to_clients(output);
            printf("hum sent to client");
        }
    }else{
        printf("hum parsing failed");
        return -1;
    }
    
    return 0;
}

static int dbp_update_temperature(char *mac)
{
    int value;
    char output[100];
    
    value = parsingGetIntAttr("tmp");
	if(value != INT_MIN){
		if(dbp_report_temp(value, mac, output) == 0){
			dbp_send_data_to_clients(output);
			printf("temp sent to client");
		}
	}
    
    return 0;
}

static int dbp_get_index_number(void)
{
    int i;
    int index = 0;
    
    pthread_mutex_lock(&dbp_mutex);
    
    for(i=0; i<MAX_NUM_OF_CONN; i++){
        if(connection.sock_fd[i] == 0){
            index = i;
            break;
        }
    }
    
    if(i >= MAX_NUM_OF_CONN){
        index = -1;
    }
    
    pthread_mutex_unlock(&dbp_mutex);
    
    return index;
}

static void dbp_add_socket(int sock_fd, int index)
{
    pthread_mutex_lock(&dbp_mutex);
    
    if(connection.conn_number <= MAX_NUM_OF_CONN){
        connection.sock_fd[index] = sock_fd;
        connection.conn_number++;
        connection.connected = true;
    }else{
        printf("%s: conn_number = %d. error, reached max number of connection\n", __func__, connection.conn_number);
    }
    
    pthread_mutex_unlock(&dbp_mutex);
}

static void dbp_remove_socket(int index)
{
    pthread_mutex_lock(&dbp_mutex);
    
    connection.sock_fd[index] = 0;
    if(connection.conn_number > 0){
        connection.conn_number--;
        if(connection.conn_number <= 0){
            connection.connected = false;
        }
    }else{
        printf("%s: conn_number = %d. error, no connection\n", __func__, connection.conn_number);
    }
    
    pthread_mutex_unlock(&dbp_mutex);
}

static int dbp_get_socket(int index)
{
    return connection.sock_fd[index];
}

void dbp_external_send_data_to_clients(char *data)
{
    dbp_send_data_to_clients(data);
}

static int dbp_send_data_to_clients(char *data)
{
    int i, j;
    int bytes_sent = 0;
    
    if(data == NULL) return -1;
    
    if(connection.connected == false) return -1;
    
    pthread_mutex_lock(&dbp_mutex);
    
    for(i=0, j=0; i<MAX_NUM_OF_CONN; i++){
        if(connection.sock_fd[i] != 0){
            j++;
            bytes_sent = send(connection.sock_fd[i], data, strlen(data)+1, 0);
            printf("%s: %d bytes sent\n", __func__, bytes_sent);
            usleep(1000); /* 1ms delay */
            if(j >= connection.conn_number){
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&dbp_mutex);
    
    return 0;
}
