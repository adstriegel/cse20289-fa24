//  Go Irish client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define ZMQ_SERVER_PORT 40000

int main (void)
{
    int rc;             /* Save the return code */

    printf ("Connecting to go irish server on port %d ... \n", ZMQ_SERVER_PORT);
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);

    char pszRequest[25];
    sprintf(pszRequest, "tcp://localhost:%d", ZMQ_SERVER_PORT);

    rc = zmq_connect (requester, pszRequest);

    if(rc == 0)
    {
        /* If this printf is printed, the network is working */
        printf("  Successfully connected on port %d\n", ZMQ_SERVER_PORT);
    }
    else 
    {
        printf("  Network connection failed\n");
        exit(-1);
    }

    int request_nbr;
    char buffer [20];

    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        printf ("Sending go %d...\n", request_nbr);
        zmq_send (requester, "go", 2, 0);
        memset(buffer, 0, 20);
        zmq_recv (requester, buffer, 20, 0);
        printf ("Received %s (Message %d)\n", buffer, request_nbr);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}
