/*
 * TellDarwin
 * Comand Interpreter for DARwIn Robot
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 *
 */

#include "src/TellDarwin.h"
#include <stdio.h>
using namespace Robot;

namespace Global
{
  bool robotMode  = true;
  int  verbosity  = 1;
}

void help(void);

int main(int argc, char* const argv[])
{
  Interpreter      interpreter;
  RequestListener* listener;
  bool shouldInitialize = true;
  char whichListener;
  char option;
  int  port = 7400;
  
  while((option = getopt(argc, argv, "hlmnsp:qv")) != -1)
    {
      switch(option)
        {
          case 'l': shouldInitialize  = false  ; Global::robotMode = false; break;
          case 'm': //cascade
          case 'n': //cascade
          case 's': whichListener     = option ; break;
          case 'p': port = atoi(optarg)        ; break;
          case 'q': Global::verbosity = 0      ; break;
          case 'v': Global::verbosity = 2      ; break;
          case 'h': //cascade
          case '?': //cascade
          default : help(); exit(0);
        }
  }
  
  if(shouldInitialize) Performer::GetInstance()->Initialize();
  
  switch(whichListener)
    {
      case 'n': listener = new NetcatRequestListener; break;
      case 's': listener = new StdIORequestListener ; break;
      case 'm': //cascade
      default : listener = new MAXRequestListener   ; break;
    } 
  listener->port = port;
  
  if(Global::verbosity >= 1)
    { 
      system("ifconfig  | grep 'inet '| grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}'");
      printf("port: %i\n", port);
    }
  
  interpreter.ListenForRequests(listener);
  delete listener;
  
  return 0;
}


void help(void)
{
  printf("/*                                     \n");
  printf(" * TellDarwin v2.0                     \n");
  printf(" * Command Interpreter for DARwIn Robot\n");
  printf(" * Written by Michael Krzyzaniak       \n");
  printf(" * at the University of Georgia        \n");
  printf(" * Ideas for Creative Exploration (ICE)\n");
  printf(" * Summer, 2011                        \n");
  printf(" * krzyzani@uga.edu                    \n");
  printf(" * idealab.uga.edu/Projects/Darwin     \n");
  printf(" */                                    \n");
  printf("                                       \n");
  printf("options:                               \n");
  printf("\t-h     \thelp   : print this help and exit                   \n");
  printf("\t-l     \ttest   : laptop mode (do not initialize the Robot's frameworks)   \n");
  printf("\t-m     \tMAX    : listen for requests from Max/MSP (default) \n");
  printf("\t-n     \tNetcat : listen for requests from netcat            \n");
  printf("\t-s     \tstdin  : listen for requests from stdin             \n");
  printf("\t-p[int]\tport   : listen on the specified port (default 7400)\n");
  printf("\t-q     \tquiet  : suppress the printing of all messages      \n");
  printf("\t-v     \tverbose: print more messages than normal            \n");
}
