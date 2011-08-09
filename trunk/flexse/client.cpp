#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include "xhead.h"

void
bail (const char *on_what)
{
  fputs (strerror (errno), stderr);
  fputs (": ", stderr);
  fputs (on_what, stderr);
  fputc ('\n', stderr);
  exit (1);
}

int
main (int argc, char **argv) {
  int err;
  char *srvr_addr = NULL;
  int port_num;
  int len_inet;			/* length  */
  int sockfd;			/* Socket */

  if (argc == 4) {
	  srvr_addr = argv[1];
	  port_num = atoi(argv[2]);
  } else {
	  printf("usage:%s dst_ip dst_port cicle\n", argv[0]);
	  bail("try again!");
  }

  struct sockaddr_in adr_srvr;	/* AF_INET */
  len_inet = sizeof adr_srvr;
  memset (&adr_srvr, 0, len_inet);
  adr_srvr.sin_family = AF_INET;
  adr_srvr.sin_port = htons (port_num);
  adr_srvr.sin_addr.s_addr = inet_addr (srvr_addr);

  if (adr_srvr.sin_addr.s_addr == INADDR_NONE) {
	  bail ("bad address.");
  }

  sockfd = socket (PF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) bail ("socket()");

  err = connect (sockfd, (struct sockaddr *) &adr_srvr, len_inet);
  if (err == -1) bail ("connect(2)");

  int logid = 0;
  char reqbuff[100000];
  char resbuff[100000];
  memset (reqbuff, 0, sizeof(reqbuff));
  memset (resbuff, 0, sizeof(resbuff));
  xhead_t* reqxhead = (xhead_t*) reqbuff;
  xhead_t* resxhead = (xhead_t*) resbuff;

  char input[1024];
  while (fgets(input, sizeof(input), stdin))
  {
      snprintf(reqxhead->srvname, sizeof(reqxhead->srvname), "%s", "myclient");
      char* str = (char*)(reqxhead+1);
      input[strlen(input) - 1] = 0;
      strncpy(str, input, 1000);
      reqxhead->detail_len = strlen(str) + 1;

      int count = atoi(argv[3]);
      for (int i=0; i<count; i++)
      {
          reqxhead->log_id = logid++;
          int ret = 0;
          if (0 != (ret = xsend(sockfd, reqxhead, 1000)))
          {
              fprintf(stderr, "send err ret[%d] ind[%d] errno[%d] [%m]\n", ret, i, errno);
              exit(1);
          }
          memset(resxhead, 0, 100000);
          if (0 != (ret = xrecv(sockfd, resxhead, sizeof(resbuff), 1000)))
          {
              fprintf(stderr, "recv err ret[%d] ind[%d] errno[%d] [%m]\n", ret, i, errno);
              exit(1);
          }
          else
          {
              printf ("count[%u] ind[%04u] rslen[%d] logid[%u] name[%s] message[%s]\n",
                      count, i, resxhead->detail_len,resxhead->log_id, resxhead->srvname, (char*)&resxhead[1]);
          }
      }
//      for (int i=0; i<count; i++)
//      {
//          int ret = 0;
//          if (0 != (ret = xrecv(sockfd, resxhead, sizeof(resbuff), 1000)))
//          {
//              fprintf(stderr, "recv err ret[%d] ind[%d] [%m]\n", ret, i);
//              exit(1);
//          }
//          else
//          {
//              str = (char*) (resxhead+1);
////              printf ("count[%u] ind[%04u] rslen[%d] logid[%u] name[%s] message[%s]\n",
////                      count, i, resxhead->detail_len,resxhead->log_id, resxhead->srvname, str);
//          }
//      }

  }

  close (sockfd);

  return 0;
}

