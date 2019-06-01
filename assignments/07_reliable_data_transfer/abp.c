#include <stdio.h>
#include <stdlib.h> /* for malloc, free, srand, rand */
#include <string.h>

/*******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

struct event {
    float evtime;           /* event time */
    int evtype;             /* event type code */
    int eventity;           /* entity where event occurs */
    struct pkt *pktptr;     /* pointer to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/


/* Here I define some function prototypes to avoid warnings */
/* in my compiler. Also I declared these functions void and */
/* changed the implementation to match */
void init();
void generate_next_arrival();
void insertevent(struct event *p);

int seq = 0;
int ack = 0;

int checksum(int seqnum, int acknum, char payload[20])
{
    int sum = 0;

    sum += (seqnum + acknum);

    for (int i=0; i < 20; i++) {
        sum += (int) payload[i];
    }

    printf("\t\t\tCHECKSUM: %d\n", sum);

    return sum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    printf("\t\tA_OUTPUT\n");

    /* Create a packet, with initial seq number, acknum = NULL, checksum and payload */
    struct pkt A_out;

    /* TODO: Replace with call to get checksum on ack, seq and payload */

    A_out.seqnum = seq;
    A_out.acknum = ack;
    A_out.checksum = checksum(seq, ack, message.data);

    size_t dest_size = sizeof (message.data);
    strncpy(A_out.payload, message.data, dest_size);

    /* Send packet to B */
    tolayer3(0, A_out);

    /* Alternate seq and ack numbers between 0 and 1 */
    seq = 1 - seq;
    ack = 1 - ack;
}


/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{

}

/* called when A's timer goes off */
void A_timerinterrupt()
{

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */
void B_output(struct msg message)  /* need be completed only for extra credit */
{

}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    printf("\t\tB_INPUT ack: %d, seq: %d, checksum: %d\n", packet.acknum, packet.seqnum, packet.checksum);

    /* TODO: Verify checksum on received packets */

    tolayer5(1, packet.payload);
}

/* called when B's timer goes off */
void B_timerinterrupt()
{
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
}


/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the transmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOULD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you definitely should not have to modify
******************************************************************/


struct event *evlist = NULL;   /* the event list */


/* possible events: */
#define     TIMER_INTERRUPT     0
#define     FROM_LAYER5         1
#define     FROM_LAYER3         2

#define     OFF                 0
#define     ON                  1
#define     A                   0
#define     B                   1


int     TRACE       = 3;         /* debugging level */
int     nsim        = 0;         /* number of messages from 5 to 4 so far */
int     nsimmax     = 10;        /* number of msgs to generate, then stop */
float   lossprob    = 0.0;       /* probability that a packet is dropped  */
float   corruptprob = 0.0;       /* probability that one bit is packet is flipped */
float   lambda      = 1000.00;   /* arrival rate of messages from layer 5 */
float   time;                    /* event time */
int     ntolayer3;               /* number sent into layer 3 */
int     nlost;                   /* number lost in media */
int     ncorrupt;                /* number corrupted by media*/


int main()
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;

   int i,j;
   int terminate = 0;

   init();
   A_init();
   B_init();

   while (1) {
        eventptr = evlist;            /* get next event to simulate */

        /* all done with simulation */
        if ( nsim == ( nsimmax )) {
            printf("\n-----------------------------------------------------------\n\n");
            terminate = 1;
            break;
        }

        if (eventptr==NULL) {
            printf("Event pointer is null.\n");
            terminate = 1;
            break;
        }

        evlist = evlist->next;        /* remove this event from event list */

        if (evlist!=NULL) {
           evlist->prev=NULL;
        }

        if (TRACE>=2) {
           printf("\nEVENT time: %f,",eventptr->evtime);
           printf("  type: %d",eventptr->evtype);
            if (eventptr->evtype==0) {
                printf(", timerinterrupt  ");
            }
            else if (eventptr->evtype==1) {
                printf(", fromlayer5\n");
            }
            else {
                printf(", fromlayer3 ");
                printf(" entity: %d\n",eventptr->eventity);
            }
        }

        /* update time to next event time */
        time = eventptr->evtime;

        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i=0; i<20; i++) {
                msg2give.data[i] = 97 + j;
            }
            if (TRACE>2) {
                printf("\tMAINLOOP: data given to student: ");
                for (i=0; i<20; i++) {
                    printf("%c", msg2give.data[i]);
                }
               printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A) {
                A_output(msg2give);
            }
            else {
                B_output(msg2give);
            }
        }
        else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;

            for (i=0; i<20; i++) {
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            }

            if (eventptr->eventity ==A) {      /* deliver packet by calling */
                A_input(pkt2give);            /* appropriate entity */
            }
            else {
                B_input(pkt2give);
                free(eventptr->pktptr);          /* free the memory for packet */
            }
        }
        else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A) {
                A_timerinterrupt();
            }
            else {
                B_timerinterrupt();
            }
        }
        else {
            printf("INTERNAL PANIC: unknown event type \n");
        }

        free(eventptr);
    }  /* End of while loop */

    if (terminate == 1) {
        printf("Simulator terminated at time %f after sending %d msgs from layer5.\n\n", time, nsim);
    }

    exit(0);
}


/* initialize the simulator */
void init() {
    int i;
    float sum, avg;
    float jimsrand();

    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    /*printf("Enter the number of messages to simulate: ");*/
    printf("Messages to simulate:                        %d\n", nsimmax);
    /*scanf("%d",&nsimmax);*/
    /*printf("Enter packet loss probability [0.0 for no loss]: ");*/
    printf("Packet loss probability:                     %f\n", lossprob);
    /*scanf("%f",&lossprob);*/
    /*printf("Enter packet corruption probability [0.0 for no corruption]: ");*/
    printf("Packet corruption probability:               %f\n", corruptprob);
    /*scanf("%f",&corruptprob);*/
    /*printf("Enter average time between messages from sender's layer5 [ > 0.0]: ");*/
    printf("Time between messages from sender's layer5:  %f\n", lambda);
    /*scanf("%f",&lambda);*/
    /*printf("Enter TRACE: ");*/
    /*scanf("%d",&TRACE);*/
    printf("Debug level:                                 %d\n\n", TRACE);
    printf("-----------------------------------------------------------\n\n");
    printf("Press enter key to continue. ");
    getchar();
    printf("\n-----------------------------------------------------------\n\n");

    srand(9999);              /* init random number generator */
    sum = (float)0.0;         /* test random number generator for students */
    for (i=0; i<1000; i++) {
        sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
    }
    avg = sum/(float)1000.0;
    if (avg < 0.25 || avg > 0.75) {
        printf("It is likely that random number generation on your machine\n" );
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(0);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time=(float)0.0;         /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}


/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/

float jimsrand()
{
    double mmm = RAND_MAX;      /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
    float x;                    /* individual students may need to change mmm */
    x = (float)(rand()/mmm);    /* x should be uniform in [0,1] */
    return(x);
}


/********************* EVENT HANDLING ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/


void generate_next_arrival()
{
    double x;
    struct event *evptr;

    if (TRACE>2) {
        printf("\tGENERATE NEXT ARRIVAL: creating new arrival\n");
    }

    x = lambda*jimsrand()*2;    /* x is uniform on [0,2*lambda] */
                                /* having mean of lambda        */

    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime =  (float)(time + x);
    evptr->evtype =  FROM_LAYER5;

    if (BIDIRECTIONAL && (jimsrand()>0.5)) {
        evptr->eventity = B;
    }
    else {
        evptr->eventity = A;
    }

    insertevent(evptr);
}


void insertevent(struct event *p)
{
    struct event *q,*qold;

    if (TRACE>2) {
        printf("\tINSERTEVENT: time is %lf\n",time);
        printf("\tINSERTEVENT: future time will be %lf\n",p->evtime);
    }

    q = evlist;     /* q points to header of list in which p struct inserted */
    if (q==NULL) {  /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
    }
    else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next) {
            qold=q;
        }
        if (q==NULL) {          /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q==evlist) {   /* front of list */
            p->next=evlist;
            p->prev=NULL;
            p->next->prev=p;
            evlist = p;
        }
        else {                  /* middle of list */
            p->next=q;
            p->prev=q->prev;
            q->prev->next=p;
            q->prev=p;
        }
    }
}


void printevlist()
{
    struct event *q;

    printf("--------------\nEvent List Follows:\n");

    for(q = evlist; q!=NULL; q=q->next) {
        printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }

    printf("--------------\n");
}


/********************** STUDENT-CALLABLE ROUTINES ***********************/


/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB) /* A or B is trying to stop timer */
{
    struct event *q;

    if (TRACE>2) {
        printf("\tSTOP TIMER: stopping timer at %f\n",time);
    }

    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q=evlist; q!=NULL; q = q->next) {
        if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
        /* remove this event */
            if (q->next==NULL && q->prev==NULL) {
                evlist=NULL;        /* remove first and only event on list */
            }
            else if (q->next==NULL) { /* end of list - there is one in front */
                q->prev->next = NULL;
            }
            else if (q==evlist) {   /* front of list - there must be event after */
                q->next->prev=NULL;
                evlist = q->next;
            }
            else {                  /* middle of list */
                q->next->prev = q->prev;
                q->prev->next =  q->next;
            }

        free(q);
        return;
        }
    }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void starttimer(int AorB, float increment) /* A or B is trying to stop timer */
{
    struct event *q;
    struct event *evptr;

    if (TRACE>2) {
        printf("\tSTART TIMER: starting timer at %f\n",time);
    }

    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q=evlist; q!=NULL ; q = q->next) {
        if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }
    }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime =  (float)(time + increment);
    evptr->evtype =  TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}


/************************** TOLAYER3 ***************/


void tolayer3(int AorB, struct pkt packet) /* A or B is trying to stop timer */
{
    struct pkt *mypktptr;
    struct event *evptr,*q;
    float lastime, x, jimsrand();
    int i;

    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)  {
        nlost++;

        if (TRACE>0) {
            printf("\tTOLAYER3: packet being lost\n");
        }
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;

    for (i=0; i<20; i++) {
        mypktptr->payload[i] = packet.payload[i];
    }

    if (TRACE>2) {
        printf("\tTOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
        mypktptr->acknum,  mypktptr->checksum);

        for (i=0; i<20; i++) {
            printf("%c",mypktptr->payload[i]);
        }
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype =  FROM_LAYER3;       /* packet will pop out from layer3 */
    evptr->eventity = (AorB+1) % 2;     /* event occurs at other entity */
    evptr->pktptr = mypktptr;           /* save ptr to my copy of packet */

    /* finally, compute the arrival time of packet at the other end.
    medium can not reorder, so make sure packet arrives between 1 and 10
    time units after the latest arrival time of packets
    currently in the medium on their way to the destination */
    lastime = time;

    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q=evlist; q!=NULL ; q = q->next) {
        if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) {
            lastime = q->evtime;
        }
    }

    evptr->evtime =  lastime + 1 + 9*jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob) {
        ncorrupt++;
        if ((x = jimsrand()) < .75) {
            mypktptr->payload[0]='Z';   /* corrupt payload */
        }
        else if (x < .875) {
            mypktptr->seqnum = 999999;
        }
        else {
            mypktptr->acknum = 999999;
        }

        if (TRACE>0) {
            printf("\tTOLAYER3: packet being corrupted\n");
        }
    }

    if (TRACE>2) {
        printf("\tTOLAYER3: scheduling arrival on other side\n");
    }

    insertevent(evptr);
}


void tolayer5(int AorB, char datasent[20])
{
    int i;

    if (TRACE>2) {
        printf("\tTOLAYER5: data received: ");
    }

    for (i=0; i<20; i++) {
        printf("%c",datasent[i]);
    }

    printf("\n");
}