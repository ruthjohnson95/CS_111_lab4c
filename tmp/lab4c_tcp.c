#include "mraa.h"
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

const int B = 4275;
const int R0 = 100;
const int button_pin = 3;
int fp;
int make_reports=1;

int celcius=0; // default: celcius; alt: F
int logflag=0;
int period = 1;
char* filename;
int sockfd;
char* id_number;
char* host_name;
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

  dprintf(sockfd, "%02d:%02d:%02d SHUTDOWN\n",hour, min, sec);
  if(logflag)
    {
      dprintf(fp, "%02d:%02d:%02d SHUTDOWN\n",hour, min, sec);
    }
  exit(0);
}

void set_args(int argc, char **argv)
{
  while(1){

    static struct option args[] = {
      {"id", required_argument, 0, 'i'},
      {"host", required_argument, 0, 'h'},
      {"log", required_argument,0, 'l'}
    };

    int option_index=0;
    int c;

    // get options from command line
    c = getopt_long(argc, argv, "ihl:h:i:l",
                    args, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    int i;

    switch(c)
      {
      case 'i': // id number
        if(strlen(optarg) == 9)
	  {
	    id_number=optarg;
	    fprintf(stderr, "Reading in ID...%s\n",optarg);
	  }
        else
	  {
	    fprintf(stderr, "Error: Invalid id number option");
	    exit(1);
	  }
        break;

      case 'h': // host name
	host_name = optarg;
	fprintf(stderr,"hostname: %s\n", host_name);
        break;

      case 'l': // log
        filename = optarg;
	fprintf(stderr,"log file: %s\n",filename);
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

  int  portno, n;

  /* set the command line args */
  set_args(argc, argv);

  /* get port number */
  portno = atoi(argv[4]);

  /* TCP Connection */
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
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_addr.s_addr = inet_addr(host_name);

  /* connect to server */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    exit(1);
  }

  if(logflag)
    {
      fp = open(filename, O_CREAT | O_WRONLY | O_NONBLOCK, S_IREAD | S_IWRITE);
    }

  char* test_buf="ID=314159265\n";
  test_buf="ID=314159265\n";

  char* id_prefix="ID=";
  dprintf(sockfd,"ID=%s\n",id_number);

  if(logflag)
    {
      dprintf(fp,"ID=%s\n",id_number);
    }

  FILE* sockfp = fdopen(sockfd,"r");

  fprintf(stderr,"ID number: %d\n", id_number);

  //  FILE* sockfp = fdopen(sockfd,"r");

    mraa_aio_context adc_a0;
  //  mraa_gpio_context gpio;

  uint16_t adcValue = 0;
  float adc_value_float = 0.0;
  adc_a0 = mraa_aio_init(0);

  int FLAG = 1;


  if (adc_a0 == NULL) {
    return 1;
  }


  //  char* buffer;
  //size_t bufsize = 32;
  size_t characters;
  //buffer = (char *)malloc(bufsize * sizeof(char));

  //gpio = mraa_gpio_init(button_pin);
  //mraa_gpio_dir(gpio, MRAA_GPIO_IN);


  struct pollfd fds;
  int ret;
  fds.fd = sockfd; /* this is STDIN */
  fds.events = POLLIN | POLLHUP | POLLERR;

  while(1){
  //while(!mraa_gpio_read(gpio)){ // while button is not pressed

    /* button reading */
    //int button_value = mraa_gpio_read(gpio);

    /* Calculate temperature reading */
    adcValue = mraa_aio_read(adc_a0);
    float R;
    R = 1023.0/((float)adcValue)-1.0;
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

       	fprintf(stderr, "%02d:%02d:%02d %0.1f\n",hour, min, sec, temp);

	dprintf(sockfd, "%02d:%02d:%02d %0.1f\n",hour, min, sec, temp);

	if(logflag)
	  {
	    dprintf(fp, "%02d:%02d:%02d %0.1f\n",hour, min, sec, temp);
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
	char* buffer;
	size_t bufsize = 32;
	buffer = (char *)malloc(bufsize * sizeof(char));

	characters = getline(&buffer, &bufsize, sockfp);

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
