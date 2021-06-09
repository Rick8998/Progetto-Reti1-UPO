#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

int VarMediaCalc = 0;

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[256] = "";
    char message[256] = "";
    struct sockaddr_in simpleServer;
    int nMessages = 0;

    if (3 != argc) {

        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    /* Store number of ineractions */
    /*do{
        printf("Insert the number of messages to send (max 10): ");
        scanf("%d", &nMessages);
    }
    while(nMessages < 1 || nMessages > 10);

    printf("Sending %d messages\n", nMessages);*/

    /* create a streaming socket      */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {

        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else
        fprintf(stderr, "Socket created!\n");

    /* retrieve the port number for connecting */
    simplePort = atoi(argv[2]);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */
    //bzero(&simpleServer, sizeof(simpleServer)); 
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    //inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
    simpleServer.sin_addr.s_addr=inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0){
	    fprintf(stderr, "Connect successful!\n");
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));
        if ( returnStatus > 0 ){
        	if(strncmp("S: OK START", buffer, 11) == 0){
            	printf("S: %s\n", buffer + strlen(buffer) - 29);
        	}
        }else{
        	fprintf(stderr, "Return Status: %d \n", returnStatus);
        }
            
    }
    else {
        fprintf(stderr, "Could not connect to address!\n");
    	close(simpleSocket);
    	exit(1);
    }

    printf("\nQUESTO PROGRAMMA CALCOLA LA MEDIA E LA VARIANZA DI VALORI INTERI PASSATI IN INPUT DA TASTIERA DALL'UTENTE\n");
    printf("\n-LA STRUTTURA DEI MESSAGGI E': <Numero_dati> <dato1> <dato2> ... <datoN>\n");
    printf("-I DATI DEVONO ESSERE TUTTE CIFRE INTERE E NON SONO AMMESSI CARATTERI ALFABETICI O SPECIALI\n");
    printf("-IL PRIMO VALORE DEVE ESSERE UNA CIFRA INTERA, NON UNO SPAZIO E NEMMENO UN CARATTERE (LETTERALE O SPECIALE)\n");
    printf("-IL PRIMO VALORE RAPPRESENTA I DATI CHE SI VOGLIONO INVIARE, IN QUELLO SPECIFICO MESSAGGIO, AL SERVER\n");
    printf("-I RESTANTI DATI SONO I VALORI DI CUI SI VUOLE CALCOLARE MEDIA E VARIANZA, E DEVONO ESSERE INTERVALLATI DA UNO SPAZIO\n");
    printf("-TUTTI I VALORI SONO INTERVALLATI DA UN CARATTERE DI 'SPAZIO'\n");
    printf("-IL MESSAGGIO DEVE TERMINARE CON UN DATO INTERO, NON CON UNO SPAZIO\n");
    printf("\n-I DATI POSSONO ESSERE MANDATI SINGOLARMENTE O A GRUPPI:\n    *IN ENTRAMBI I CASI BISOGNA SEMPRE SPECIFICARE LA QUANTITA' DI DATI CHE SI STANNO INVIANDO AL SERVER\n");
    printf("    *QUINDI SE SI VUOLE MANDARE UN SOLO DATO LA STRUTTURA SARA': 1 <dato>\n    *CON PIU' DATI LA STRUTTURA E' QUELLA DESCRITTA PRECEDENTEMENTE");
    printf("\n-TERMINATA LA SCRITTURA DEI DATI, PREMERE 'INVIO' PER INVIARE I DATI AL SERVER\n");
    printf("-E' POSSIBILE INVIARE PIU' MESSAGGI: IL SERVER SALVERÀ I DATI NECESSARI PER IL CALCOLO DI MEDIA E VARIANZA\n");
    printf("-UNA VOLTA CHE SONO STATI INVIATI TUTTI I DATI DI CUI SI VUOLE CALCOLARE MEDIA E VARIANZA, BISOGNA INVIARE AL SERVER IL SOLO NUMERO 0 (ZERO)\n");
    printf("-TRAMITE QUESTO NUMERO IL SERVER RICONOSCERA' CHE L'UTENTE VUOLE CALCOLARE MEDIA E VARIANZA DEI DATI INVIATI FINO A QUEL MOMENTO\n");
    printf("-0 (ZERO) PU0' ESSERE INSERITO TRA I DATI DI CUI SI VUOLE CALCOLARE MEDIA E VARIANZA, PURCHE' LA STRUTTURA DEL MESSAGGIO RISPETTI LE REGOLE DESCRITTE PRECEDENTEMENTE\n");
    
    do{

        //Take the message to send to the server 
    	do{
    		printf("\n");
	        {
	            fflush(stdin);
	            char *pin;
	            printf("INSERISCI IL MESSAGGIO: ");
	            fgets(message, 256, stdin);
	            pin = strrchr(message, '\n');
	            if(pin != NULL)
	                *pin = '\0';
	            //printf("The message sent is: %s\n", message);
	        }

	        //strcat(message, "\n");


	        memset(buffer, 0, sizeof(buffer));
	        //send the message to the server 
	        returnStatus = write(simpleSocket, message, strlen(message));
	        if(returnStatus <= 0){
	            //fprintf(stderr, "Return Status = %d Error %d (%s)\n", returnStatus, errno, strerror(errno));
	            printf("INSERIRE DEI DATI!!!!!!!!!\n");
	        }
	    }while(returnStatus <= 0);

         //get the message from the server   
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));
        if ( returnStatus > 0 ){
        	if(strncmp("S: OK DATA", buffer, 10) == 0){
        		printf("I DATI SONO STATI INVIATI CORRETTAMENTE AL SERVER E SONO STATI SALVATI\n");
        	}
        	else if(strncmp("S: OK STATS", buffer, 11) == 0){
        		printf("CALCOLO MEDIA E VARIANZA\n");
        		printf("\nLA MEDIA E LA VARIANZA VALGONO RISPETTIVAMENTE:\n");
        		for(int y = 12; y < 256; y++){
        			if((isspace(buffer[y]))  > 0){
		        		char charMediaAry[256];
		        		int k = 0;
		        		int i;
		        		for(i = y+1; i < 256; i++){
		        			if((isdigit(buffer[i])) > 0 || (ispunct(buffer[i])) >0){
		        				charMediaAry[k] = buffer[i];
		        				k++;
		        			}
		        			else break;
		        		}
		        		float media = atof(charMediaAry);
		        		printf("MEDIA = %f\n", media);

		        		char charVarAry[256];
		        		int j = 0;
		        		for(int z = i; z < 256; z++){
		        			if((isspace(buffer[z]))  > 0){;
		        				for(int x = z+1; x < 256; x++){
		        					if((isdigit(buffer[x])) > 0 || (ispunct(buffer[x])) >0){
			        					charVarAry[j] = buffer[x];
			        					j++;
			        				}
			        				else break;
		        				}
		        				break;
		        			}
		        		}
		        		float varianza = atof(charVarAry);
		        		printf("VARIANZA = %f\n", varianza);
		        		
		        		VarMediaCalc = 1;
		        		if(VarMediaCalc == 1){
		        			break;
		        		}
	        		}
	        		else if(VarMediaCalc == 1){
	        			break;
	        		}	
	        	}
	        	break;
        	}
        	else if(strncmp("S: ERR DATA", buffer, 11) == 0){
        		printf("ERRORE: IL NUMERO DI DATI INVIATI NON E' COERENTE CON QUELLO DICHIARATO COME PRIMO ELEMENTO DEL MESSAGGIO\n");
        		break;
        	}
        	else if(strncmp("S: ERR STATS", buffer, 12) == 0){
        		printf("ERRORE: LA MEDIA E LA VARIANZA NON POSSONO ESSERE CALCOLATE CORRETTAMENTE\n");
        		break;
        	}
        	else if(strncmp("S: ERR SYNTAX", buffer, 11) == 0){
        		printf("ERRORE: LA SINTASSI DEL MESSAGGIO INVIATO NON E' CORRETTA (INSERIRE SOLO CIFRE NUMERICHE INTERE INTERVALLATE DA SPAZI)\n");
        		printf("NOTA: ALL'INIZIO E ALLA FINE DEL MESSAGGIO DEVE ESSERCI UNA CIFRA NUMERICA (NON UNO SPAZIO)\n");
        		break;
        	}
        	else {
        		printf("ERRORE: QUALCOSA E' ANDATO STORTO\n");
        		break;
        	}
        	//printf("%s", buffer);

    	}
        else{
            fprintf(stderr, "Return Status: %d \n", returnStatus);
        	break;
        }

        /*if(strncmp("S: ERR DATA", buffer, 11) == 0){
        	break;
        }*/
        //close(simpleSocket)l;
    }while(atoi(message) != 0);

    close(simpleSocket);
    return 0;
}