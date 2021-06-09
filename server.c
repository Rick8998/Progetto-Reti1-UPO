#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

int checkSyntax(char msg[]);//prototipo
char auxToInt[512]; /*array ausiliario per la memorizzazione dei dati in charToIntAry[] (e quindi la 
conversione di ogni carattere char in int)*/
int charToIntAry[512]; /*array di elementi interi [il primo elemento è numEL, i restanti sono i valori di
cui bisogna calcolare media e varianza]*/

int counterNumEl = -1; /*conta il numero di valori di cui si deve fare la media (i valori TOTALI ricevuti
fino ad ora) (è != NumEl, perchè uno memorizza il primo elemento del buffer (numEL), l'altro (counterNumEl) 
conta i valori per cui bisogna fare la media. Ciò serve per verificare che i due valori effettivamente 
corrispondano)----inizia da -1 perchè nel conteggio fatto nel if(isdigit(msg[i]) == 0) della checkSyntax
devo saltare il primo elemento del buffer (cioè numEL) che salvo in posizione 0 di charToIntAry*/
int numDatiLetti = 0; //conta il numero di dati all'interno del buffer
int numEl; //memorizza il primo valore del buffer (il numero di dati del messaggio appena ricevuto)
float sum = 0; //somma di tutti i valori per il calcolo della media
float sumVariance; // somma di tutti i valori per il calcolo della varianza
float varianza; // varianza
float media = 0; //media
int firstMessage = 0; /*se il client ha già mandato un messaggio, il server non deve considerare il primo
elemento del prossimo messaggio come uno degli elementi di cui fare la media(0 = il messaggio è il primo 
che il server riceve, 1 = il server ha già ricevuto altri messaggi dal client*/
int actualCounterNumEL = -1; /*conta gli elementi del messaggio appena ricevuto (è != counterNumEl, perchè
counterNumEl conta il numero di valori TOTALI, quindi considerando anche gli eventuali messaggi 
precedenti)*/
int indexCharInt = 0; //indice dell'array di interi charToIntAry

int main(int argc, char *argv[]) {
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    //time_t ticks = time(NULL);
    char buffer [512] = ""; //punto 2
    //int MaxNumber = 0;

    if (2 != argc) {
        fprintf(stderr, "$ programma_server <numero porta>\n"); //punto 1
        exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else {
	    fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

    /* setup the address structure */
    /* use INADDR_ANY to bind to all local addresses  */
    memset(&simpleServer, '\0', sizeof(simpleServer)); 
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /*  bind to the address and port with our socket  */
    returnStatus = bind(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));

    if (returnStatus == 0) {
	    fprintf(stderr, "Bind completed!\n");
    }
    else {
        fprintf(stderr, "Could not bind to address!\n");
	close(simpleSocket);
	exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1) {
        fprintf(stderr, "Cannot listen on socket!\n");
	   close(simpleSocket);
        exit(1);
    }

    while (1){
        int chiudiConnessione = 0; //flag per verificare quando il server deve chiudere la connessione con il client
        struct sockaddr_in clientName = { 0 };
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);
        
		/* wait here */
	    simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, (socklen_t *)&clientNameLength);
		if (simpleChildSocket == -1) {
            memset(buffer, 0, sizeof(buffer));
	        fprintf(stderr, "Cannot accept connections!\n");
            snprintf(buffer, sizeof(buffer), "S: ERR START");
            write(simpleChildSocket, buffer, strlen(buffer));
		    close(simpleSocket);
		    exit(1);
		}
        else{
            //OK START
            memset(buffer, 0, sizeof(buffer));
            snprintf(buffer, sizeof(buffer), "S: OK START, BENVENUTO INVIAMI I TUOI DATI");
            write(simpleChildSocket, buffer, strlen(buffer));
            memset(buffer, 0, sizeof(buffer));
        }

    	do{
            memset(buffer, 0, sizeof(buffer));
    		/* get the message from the client   */
    	    returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
            //punto 5->verifica correttezza sintattica dei messaggi in arrivo dal client
    	    if ( returnStatus > 0 ){ // verifa della read
                //--punto 5->verifica correttezza sintattica dei messaggi in arrivo dal client
                if(checkSyntax(buffer) == 1 ){

                   // --punto 6a-> Se il numero di dati letti è corretto e maggiore di zero, memorizza i dati e risponde OK
                    if(numDatiLetti > 0 && numEl == actualCounterNumEL && numEl != 0){

                        snprintf(buffer, sizeof(buffer), "S: OK DATA %d\n", numEl);
                        write(simpleChildSocket, buffer, strlen(buffer));
                        memset(buffer, 0, sizeof(buffer));
                        firstMessage = 1;
                        //salvaDatiLetti
                        //OK DATA <numero_dati_letti>
                    }
                    else if(numDatiLetti > 0  && numEl != actualCounterNumEL /*&& numEl != 0*/){
                       
                        snprintf(buffer, sizeof(buffer), "S: ERR DATA, NUMERO DATI NON COERENTE\n");
                        write(simpleChildSocket, buffer, strlen(buffer));
                        memset(buffer, 0, sizeof(buffer));
                        /*--punto 6b SERVER -> Se il numero di dati letti non è corretto, ovvero il valore dichiarato non è coerente con il contenuto del messaggio, il server risponde ERR
                        ERR DATA <messaggio> e chiude la connessione*/
                        memset(charToIntAry, '\0', sizeof(auxToInt)); //scarto i dati errati
                        chiudiConnessione = 1;
                    }
                    else if(numEl == 0 && actualCounterNumEL == 0){
                        if(counterNumEl > 1){
                            /*--punto 6c-i -> Se media e varianza dei campioni possono essere calcolati correttamente, il server trasmette OK
                            calcola media e varianza dei dati memorizzati fin_ora
                            OK STATS <numero_totale_di_campioni> <media> <varianza>*/
                            
                            //calcolo media
                            for (int i = 1; i <= counterNumEl; i++)
                            {
                                sum += charToIntAry[i];
                            }
                            media = sum / counterNumEl;

                            //calcolo varianza
                            for (int i = 1; i <= counterNumEl; i++){
                                sumVariance = sumVariance + pow((charToIntAry[i] - media), 2);
                            }

                            varianza = sumVariance / counterNumEl;

                            snprintf(buffer, sizeof(buffer), "S: OK STATS %d %f %f\n", counterNumEl, media, varianza);
                            write(simpleChildSocket, buffer, strlen(buffer));
                            memset(buffer, 0, sizeof(buffer));
                            chiudiConnessione = 1;
                        }
                        else{
                            /*--punto 6c-ii -> Se media o varianza dei campioni non possono essere calcolati correttamente, il server trasmette ERR
                            ERR STATS <messaggio>*/
                            snprintf(buffer, sizeof(buffer), "S: ERR STATS, MEDIA O VARIANZA DEI CAMPIONI NON POSSONO ESSERE CALCOLATE CORRETTAMENTE\n");
                            write(simpleChildSocket, buffer, strlen(buffer));
                            memset(buffer, 0, sizeof(buffer));
                            chiudiConnessione = 1;
                        }
                        
                    }
                    else{
                        snprintf(buffer, sizeof(buffer), "S: ERR MESSAGGIO NON CORRETTO\n");
                        write(simpleChildSocket, buffer, strlen(buffer));
                        memset(buffer, 0, sizeof(buffer));
                        memset(charToIntAry, '\0', sizeof(auxToInt)); //scarto i dati errati
                    }
                }
                else{
                    //messaggio scorretto-> write-> ERR SYNTAX <MESSAGGIO>
                    //ERR SYNTAX Sintassi errata
                    snprintf(buffer, sizeof(buffer), "S: ERR SYNTAX, SINTASSI MESSAGGIO NON CORRETTA\n");
                    write(simpleChildSocket, buffer, strlen(buffer));
                    memset(buffer, 0, sizeof(buffer));
                }
                
            }
    	    else{
                chiudiConnessione = 1;
    	        fprintf(stderr, "Read fallita--Return Status 4 = %d Error %d (%s)\n", returnStatus, errno, strerror(errno));
            }
            memset(buffer, 0, sizeof(buffer));
        }
        while(chiudiConnessione == 0);
        
        //il server ritorna allo stato iniziale (per essere pronto in caso di nuova connessione)
        memset(buffer, 0, sizeof(buffer));
        numEl = 0;
        counterNumEl = -1;
        numDatiLetti = 0;
        sum = 0;
        media = 0;
        varianza = 0;
        sumVariance = 0;
        firstMessage = 0;
        indexCharInt = 0;
        memset(charToIntAry, '\0', sizeof(auxToInt));
        memset(auxToInt, '\0', sizeof(auxToInt));
        //chiusura socket
        close(simpleChildSocket);
        printf("\nChiudo simpleChildSocket\n");
    }

    close(simpleSocket);
    printf("Ciudo simpleSocket\n");
    return 0;
}


/*funzione che controlla la sintassi del messaggio sia corretta
ritorna 1 se il messaggio è corretto, 0 altrimenti*/
int checkSyntax(char msg[]){
    int indexAuxToInt = 0; //indice dell'array ausiliario auxToInt
    actualCounterNumEL = -1;
    numEl = atoi(msg); //il <numero_dati> passato dal client è memorizzato in posizione 0 del buffer

    if(strlen(msg) == 0){
        return 0;
    }

    for(int i = 0; i <= strlen(msg); i++){
        if(isspace(msg[strlen(msg)-1]) > 0){
            return 0;
        }
        else if (ispunct(msg[i]) > 0)
        {
            return 0;
        }
        else if((isspace(msg[0]) > 0)){
             /*il server accetta solo valori interi o spazi, e al primo elemento deve esserci per forza un
            valore intero)*/
            return 0;
        }
        else if((isspace(msg[i - 1]) >0) && (isspace(msg[i]) >0)){
            //il client non può inviare un messaggio con due spazi consecutivi
            return 0;
        }
        else if(isalpha(msg[i])){
            //il client non può inviare caratteri
            return 0;
        }

        if((isdigit(msg[i]) > 0)){
            /*se c'è una cifra devo memorizzarla (se un numero è >=10 le cifre saranno memorizzate in più
             "slot" del buffer) quindi questa operazione deve essere ripetuta finchè non trovo uno spazio*/
            if(firstMessage == 0 || (firstMessage == 1 && i != 0)){
                /*se il client ha già mandato un messaggio, dal secondo in poi non bisogna considerare il 
                primo elemento come un elemento di cui bisogna fare la media*/
                auxToInt[indexAuxToInt] = msg[i];
                indexAuxToInt++;
            }
        }
        else if(isdigit(msg[i]) == 0 && msg[0] != 0){ 
            /*quando c'è uno spazio o \0 il numero da memorizzare è terminato->lo salvo nell'array di interi*/
            if(firstMessage == 0 || (firstMessage == 1 && i != 1)){
            /*se il client ha già mandato un messaggio, dal secondo in poi non bisogna considerare il 
            primo elemento come un elemento di cui bisogna fare la media, quindi viene "saltato" durante
            l'inserimento dei dati nell'array di interi*/
                if(atoi(msg) != 0 ){
                    //se il client manda solo 0, il counter degli elementi NON deve essere aumentato
                    counterNumEl++;
                }
                actualCounterNumEL++;
                charToIntAry[indexCharInt] = atoi(auxToInt);

                memset(auxToInt, '\0', sizeof(auxToInt)); //refresh dell'array ausiliario
                indexAuxToInt = 0;
                indexCharInt ++;
                numDatiLetti++;
            }
            else
                /*il contatore di messaggi deve essere incrementato comunque anche in caso 
                di messaggi successivi al primo, dato che parte da -1*/
                actualCounterNumEL++;
        }
        else{
            return 0;
        }
        
    }
    return 1;
}