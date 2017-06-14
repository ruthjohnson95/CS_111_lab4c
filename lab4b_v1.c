//#include "mraa.h"

#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>

#include <openssl/ssl.h>

const int B = 4275;
const int R0 = 100;
const int button_pin = 3;
int fp;
int make_reports=1;

int celcius=0; // default: celcius; alt: F
int logflag=0;
int period = 1;
char* filename;

int GO_FLAG=1;

void m_shutdown()
{
  // log time and SHUTDOWN
  time_t curtime;
  curtime = time (NULL);
  struct tm *tm_struct = localtime (&curtime);
  int hour = tm_struct -> tm_hour;
  int min = tm_struct -> tm_min;
  int sec = tm_struct -> tm_sec;

  dprintf(fp, "%02d:%02d:%02d SHUTDOWN\n",hour, min, sec);
  exit(0);
}

void set_args(int argc, char **argv)
{
  while(1){

    static struct option args[] = {
      {"scale", required_argument, 0, 's'},
      {"period", required_argument, 0, 'p'},
      {"log", required_argument,0, 'l'}
    };

    int option_index=0;
    int c;

    // get options from command line
    c = getopt_long(argc, argv, "spl:p:s:l",
                    args, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    int i;

    switch(c)
      {
      case 's': // scale
        if(optarg[0] == 'F')
        {
          celcius=0;
        }
        else if(optarg[0] == 'C')
        {
          celcius=1;
        }
        else
        {
          fprintf(stderr, "Error: Invalid scale option - %s\n",optarg);
          exit(1);
        }
        break;

      case 'p': // period
        period=atoi(optarg);
        break;

      case 'l': // log
        filename = optarg;
        logflag = 1;
        break;

      default: // wrong command-line parameters
        fprintf(stderr, "Error: Not valid argument\n");
        exit(1);

      }

  }
  // end of command-line while()

}

int main ( int argc, char **argv )
{
  /* TCP Connection */

  int sockfd,  portno, n;
  struct sockaddr_in  serv_addr;
  struct hostent *server;

  /* create a socket point */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  server = gethostbyname("lever.cs.ucla.edu");
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(18000);
  serv_addr.sin_addr.s_addr = inet_addr("131.179.192.136");

  /* connect to server */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    exit(1);
  }

  char* test_buf="ID=314159265\n";
  write(sockfd,test_buf,strlen(test_buf));

  /* SSL part */
  
  SSL *sslClient = NULL;
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  SSL_CTX *newContext = SSL_CTX_new(SSLv23_client_method());
  sslClient = SSL_new(newContext);

  SSL_set_fd(sslClient, sockfd);
  if(SSL_connect(sslClient)!=1)
    {
      fprintf(stderr,"Error: cannot connect to server\n"); 
    }

  //  SSL_write();
  //  SSL_read();
  //  SSL_shutdown(sslClient);
  //  SSL_free(sslClient);
   
  //  char* test="test\n";
  //  write(sockfd,test,strlen(test));

  //mraa_aio_context adc_a0;
  //mraa_gpio_context gpio;
  uint16_t adcValue = 0;
  float adc_value_float = 0.0;
  //adc_a0 = mraa_aio_init(0);
   
  int FLAG = 1;

  /*
  if (adc_a0 == NULL) {
    return 1;
  }
  */

  /* set the command line args */
  set_args(argc, argv);

  char* buffer;
  int bufsize = 32;
  size_t characters;
  buffer = (char *)malloc(bufsize * sizeof(char));

  //gpio = mraa_gpio_init(button_pin);
  //mraa_gpio_dir(gpio, MRAA_GPIO_IN);


  struct pollfd fds;
  int ret;
  fds.fd = sockfd; /* this is STDIN */
  fds.events = POLLIN | POLLHUP | POLLERR;

  if(logflag)
    {
      fp = open(filename, O_CREAT | O_WRONLY | O_NONBLOCK);
    }

  while(1){
  //while(!mraa_gpio_read(gpio)){ // while button is not pressed

    /* button reading */
    //int button_value = mraa_gpio_read(gpio);

    /* Calculate temperature reading */
    //adcValue = mraa_aio_read(adc_a0);
    float tmp = 3.14;
    float R;
    //R = 1023.0/((float)adcValue)-1.0;
    R = 1023.0/((float)tmp)-1.0;
    R = 100000.0*R;
    float temp  = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;

    /* Farenheit */
    if(celcius == 0)
      {
	temp = temp*(9.0/5.0) + 32;
      }

    /* Local Time */
    time_t curtime;
    curtime = time (NULL);
    struct tm *tm_struct = localtime (&curtime);
    int hour = tm_struct -> tm_hour;
    int min = tm_struct -> tm_min;
    int sec = tm_struct -> tm_sec;

    /* print logs  */
    if(make_reports)
      {
	double rand_num;
	rand_num=50.2+rand()%10;
	printf("rand num: %d\n",rand_num); 
	fprintf(stdout, "%02d:%02d:%02d ",hour, min, sec);
	fprintf (stdout, "%0.1f\n", rand_num);
	
	//	dprintf(sockfd, "%02d:%02d:%02d ",hour, min, sec);
	//	dprintf (sockfd, "%0.1f\n", rand_num);

	if(logflag)
	  {
	    dprintf(fp, "%02d:%02d:%02d ",hour, min, sec);
	    dprintf (fp, "%0.1f\n", rand_num);

	  }
      } // end of if reporting

    unsigned int retTime = time(0) + period;   // Get finishing time.
    while (time(0) < retTime )
      {
        // only do polling and button input
	/* get button state */
  /*
	if(mraa_gpio_read(gpio))
	  {
	    m_shutdown();
	  }
  */
    /* poll for input */
    ret = poll(&fds, 1, 0);

    /* check for polling errors */
    if(fds.revents & (POLLHUP+POLLERR))
      {
	fprintf(stderr, "Error in polling\n");
	exit(1);
      }

    /* read input if there */
    if(fds.revents & POLLIN)
      {
	fprintf(stderr, "Polling input...\n");
	int recieved = SSL_read(sslClient,&buffer, bufsize); 
	//characters = getline(&buffer,&bufsize,stdin);
	//	printf("You typed: %s \n",buffer);
	write(2,&buffer,bufsize); 

	if(strcmp(buffer, "OFF\n") == 0)
	  {
	    //	    fprintf(stderr, "...OFF\n");
	    if(logflag)
	      {
		dprintf(fp, "OFF\n");
	      }
	    m_shutdown();
	  }
	else if(strcmp(buffer, "STOP\n") == 0)
	  {
	    make_reports = 0;
	    if(logflag)
	      {
		dprintf(fp,"STOP\n");
	      }
	    //	    fprintf(stderr, "...STOP\n");
	  }
	else if(strcmp("START\n", buffer) == 0)
	  {
	    make_reports = 1 ;
	    //	    fprintf(stderr, "...START\n");
	    if(logflag)
	      {
		dprintf(fp, "START\n");
	      }
	  }
	else if(strcmp(buffer, "SCALE=F\n") == 0)
	  {
	    celcius=0;
	    //	    fprintf(stderr, "...SCALE=F\n");
	    if(logflag)
	      {
		dprintf(fp, "SCALE=F\n");
	      }
	  }
	else if(strcmp(buffer, "SCALE=C\n") == 0)
	  {
	    celcius=1;
	    //	    fprintf(stderr, "...SCALE=C\n");
	    if(logflag)
	      {
		dprintf(fp, "SCALE=C\n");
	      }
	  }
	else if( strstr(buffer, "PERIOD=") != NULL )
	  {
	    period = atoi(buffer+7);

	    //	    fprintf(stderr, "...PERIOD=%d\n", period);
	    if(logflag)
	      {
		dprintf(fp, "PERIOD=%d\n", period);
	      }
	  }
	else
	  {
	    fprintf(stderr, "Error: input option not valid\n");
	    exit(1);
	  }

      } // end of poll if

    } // end of only poll + button

  } // end of infinite for-loop

  //mraa_aio_close(adc_a0);

  if(logflag)
    {
      close(fp);
    }

  m_shutdown();

  return 0;
}
