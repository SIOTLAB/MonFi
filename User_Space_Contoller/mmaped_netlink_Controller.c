// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <sys/mman.h>
#include <poll.h>
#include <errno.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sched.h>


//defining netlink port for mmaped-netlink
#define NETLINK_TEST 30

#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

int  g_skfd;                                    //return value of ther socket
unsigned int g_ring_size;                       //ring buffer size
void *g_rx_ring;                                // pointers to the tx and rx ring buffers
void *g_tx_ring;
unsigned int g_tx_fr_off;                       //ring buffer offset for new messages received and sent
unsigned int g_rx_fr_off;




#define MAX_PAYLOAD 1024  /* maximum payload size */

// analogous to the debug_level in the kernel module
// if DEBUG_LEVEL == 3, print all or print nothing
#define DEBUG_LEVEL 0

#define RAW_ON 10
#define MAX_MSGSIZE 1024


// return the wallclock time in microsecond. Used for timekeeping
long int GetTimeStamp()
{
    struct timeval et;
    gettimeofday(&et, NULL);
    long int timestamp_in_micro = (et.tv_sec * 1000000) + (et.tv_usec) ;
    return timestamp_in_micro;
}


// this function helps in extract the string between two identifiers -PATTERN1 and PATTERN2.
int func1(char **arr, size_t *arr_len, char *s, char *PATTERN1, char *PATTERN2)
{
    char *target = NULL;
    char *start, *end;

    if ( start = strstr( s, PATTERN1 ) )
    {
        start += strlen( PATTERN1 );
        if ( end = strstr( start, PATTERN2 ) )
        {
            target = ( char * )malloc( end - start + 1 );
            memcpy( target, start, end - start );
            target[end - start] = '\0';
        }
    }

    if ( target )
    {
        if(DEBUG_LEVEL == 3)
            printf( "for verification from withtin the function ... func1 :%s\n", target );
    }
    *arr = target;
    *arr_len = 2;

    return 0;
}


//chops n characters from a given string
void chopN(char *str, size_t n)
{
    assert(n != 0 && str != 0);
    size_t len = strlen(str);
    if (n > len)
        return;  // Or: n = len;
    memmove(str, str + n, len - n + 1);
}





int main(int argc, char **argv)
{
    if(DEBUG_LEVEL == 3)
        printf("run....\n");
    int done_once_1 = 0;

    /* socket */
    g_skfd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_TEST);
    if(g_skfd == -1)
    {
        perror("create socket error\n");
        return -1;
    }

    // assigning the port to the socket
    struct sockaddr_nl addr =
    {
        .nl_family  = AF_NETLINK,
    };
    addr.nl_pid = getpid();

    // bind netlink socket
    if( 0 != bind(g_skfd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() error\n");
        close(g_skfd);
        return -1;
    }

    //Ring setup
    unsigned int block_size = 16 * getpagesize();
    printf("block_size=%d\n", block_size);
    struct nl_mmap_req req =
    {
        .nm_block_size    = block_size,
        .nm_block_nr    = 64,
        .nm_frame_size    = 16384,
        .nm_frame_nr    = 64 * block_size / 16384,
    };


    // Configuring ring parameters
    if (setsockopt(g_skfd, SOL_NETLINK, NETLINK_RX_RING, &req, sizeof(req)) < 0)
    {

        printf("set rx_ring erro=%d!!!\n", errno);
        exit(1);
    }
    if (setsockopt(g_skfd, SOL_NETLINK, NETLINK_TX_RING, &req, sizeof(req)) < 0)
    {
        printf("set tx_ring erro!!!\n");
        exit(1);
    }

    // Calculate size of each invididual ring
    g_ring_size = req.nm_block_nr * req.nm_block_size;
    // Map RX/TX rings. The TX ring is located after the RX ring
    g_rx_ring = mmap(NULL, 2 * g_ring_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, g_skfd, 0);
    if ((long)g_rx_ring == -1L)
        exit(1);
    g_tx_ring = g_rx_ring + g_ring_size;

    //Message transmission
    struct nl_mmap_hdr *hdr;
    struct nlmsghdr *nlh_tx;

    hdr = g_tx_ring + g_tx_fr_off;
    if (hdr->nm_status != NL_MMAP_STATUS_UNUSED)
        /* No frame available. Use poll() to avoid. */
        exit(1);

    nlh_tx = (void *)hdr + NL_MMAP_HDRLEN;

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *) & (addr);
    msg.msg_namelen = sizeof(addr);


    char buffer[100];

    int i = 0;
    double for_avg_deltat2_t2 = 0;


    if (argc != 2)
    {
        printf("usage: %s <message>\n", argv[1]);
        return 1;
    }

    char delim[] = " ";

    char *pch;
    int parameter_count = 1;
    int user_iterations = 1;
    char str[100];
    printf("Hello World\n\n");
    const char needle[100] = ":";
    char *ret;

    ret = strstr(argv[1], needle);

    strcpy(buffer, ret);
    strcpy(str, ret);
    chopN(buffer, 1);
    chopN(str, 1);



    printf("The buffer is: %s\n", buffer);
    printf("The bufferstr is: %s\n", str);

    char find = ':';

    char iterations[10];
    int ITERATIONS = 1;
    const char *ptr = strchr(argv[1], find);
    if(ptr)
    {
        int index = ptr - argv[1];
        printf("The index is: %d\n", index);
        strncpy ( iterations, argv[1],  index);
        printf("The iteratiosn is: %s\n", iterations);
        ITERATIONS = atoi(iterations);
        printf("The ITERATIONS is: %d\n", ITERATIONS);
    }





    printf ("Splitting string \"%s\" into tokens:\n", str);
    pch = strtok (str, " ,.-");
    while (pch != NULL)
    {
        printf("------::found %s::------\n", pch);
        if (parameter_count == 1)
        {
            printf ("first parameter %s\n", pch);
            pch = strtok (NULL, " ,.-");
        }
        else if(parameter_count == 2)
        {
            printf ("second parameter %s\n", pch);
            int x;
            sscanf(pch, "%d", &x);
            printf("\nThe value of x : %d", x);
            pch = strtok (NULL, " ,.-");
            user_iterations = x;
        }
        else if(parameter_count == 3)
        {
            printf ("Third parameter %s\n", pch);
            pch = strtok (NULL, " ,.-");
            break;
        }
        parameter_count++;
    }

    size_t t1, t2;
    t1 = 100000000;
    t2 = sizeof(int);
    FILE *out = fopen("filename2", "a");
    int kernel_interations = 0;


    long int start_t = GetTimeStamp();
    struct nl_mmap_hdr *hdr_rx;
    struct nlmsghdr *nlh_rx;
    unsigned char buf[16384];
    ssize_t len;
    struct pollfd pfds[1];
    while(i < ITERATIONS)
    {

        i++;

        g_tx_ring = g_rx_ring + g_ring_size;

        //Message transmission

        hdr = g_tx_ring + g_tx_fr_off;
        if (hdr->nm_status != NL_MMAP_STATUS_UNUSED)
            /* No frame available. Use poll() to avoid. */
            exit(1);



        nlh_tx = (void *)hdr + NL_MMAP_HDRLEN;

        strcpy(NLMSG_DATA(nlh_tx), buffer); //buffer contain the argv[1], input that determines the measurement collection methodology
        if(DEBUG_LEVEL == 3)
            printf("buffer: %s\n", buffer);

        nlh_tx->nlmsg_len = sizeof(*nlh_tx) + sizeof(buffer);
        nlh_tx->nlmsg_pid = getpid();  /* self pid */
        nlh_tx->nlmsg_flags = 0;
        hdr = g_tx_ring + g_tx_fr_off;
        if (hdr->nm_status != NL_MMAP_STATUS_UNUSED)
            /* No frame available. Use poll() to avoid. */
            exit(1);

        nlh_tx = (void *)hdr + NL_MMAP_HDRLEN;

        msg.msg_name = (void *) & (addr);
        msg.msg_namelen = sizeof(addr);
        if(DEBUG_LEVEL == 3)
            printf("sendto.....\n");
        /* Fill frame header: length and status need to be set */
        hdr->nm_len = nlh_tx->nlmsg_len;
        hdr->nm_status  = NL_MMAP_STATUS_VALID;
        struct sockaddr_nl nladdr =
        {
            .nl_family  = AF_NETLINK,
        };
        if(DEBUG_LEVEL == 3)
            printf("g_skfd : %d \n", g_skfd);
        int send_success = sendto(g_skfd, NULL, 0, 0, &nladdr, sizeof(nladdr));
        if(DEBUG_LEVEL == 3)
            printf("send_success : %d \n", send_success);
        if (send_success < 0)
            exit(1);

        g_tx_fr_off = (g_tx_fr_off + req.nm_frame_size) % g_ring_size;
        if(DEBUG_LEVEL == 3)
            printf("sent frame.....\n");
        if(DEBUG_LEVEL == 3)
            printf("sent frame.....1\n");
        if (done_once_1 == 0)
        {
            pfds[0].fd = g_skfd;
            pfds[0].events = POLLIN | POLLERR;
            pfds[0].revents = 0;
            done_once_1 = 1;
            printf("sent frame.....1.1\n");
        }
        else
        {
            if(DEBUG_LEVEL == 3)
                printf("sent frame.....1.2\n");
        }
        if(DEBUG_LEVEL == 3)
            printf("sent frame.....1.3\n");
        if (poll(pfds, 1, -1) < 0 && errno != -EINTR)
            exit(1);
        if(DEBUG_LEVEL == 3)
            printf("sent frame.....2\n");
        /* Check for errors. Error handling omitted */
        if (pfds[0].revents & POLLERR)
            exit(1);

        /* If no new messages, poll again */
        if (!(pfds[0].revents & POLLIN))
            exit(1);
        if(DEBUG_LEVEL == 3)
            printf("sent frame.....3\n");
        /* Process all frames */


        int survey_time_1;
        int survey_time_busy_1;
        double timestamp_1_2;
        double timestamp_1_1;
        int survey_time_2;
        int survey_time_busy_2;

        int survey_time_3;
        int survey_time_busy_3;
        int survey_time_4;
        int survey_time_busy_4;
        double timestamp_4_2;
        double timestamp_4_1;

        if(DEBUG_LEVEL == 3)
            printf("bfore  frame..while (kernel_interations < user_iterations)s...\n");
        kernel_interations = 0;
        while (kernel_interations < user_iterations)
        {
            /* Get next frame header */
            if(DEBUG_LEVEL == 3)
                printf("recev ....\n");
            hdr_rx = g_rx_ring + g_rx_fr_off;

            if (hdr_rx->nm_status == NL_MMAP_STATUS_VALID)
            {
                if(DEBUG_LEVEL == 3)
                    printf("Regular memory mapped frame.\n");
                /* Regular memory mapped frame */
                nlh_rx = (void *)hdr_rx + NL_MMAP_HDRLEN;
                len = hdr_rx->nm_len;

                /* Release empty message immediately. May happen
                 * on error during message construction.
                         */
                if (len == 0)
                    goto release;
            }
            else if (hdr_rx->nm_status == NL_MMAP_STATUS_COPY)
            {
                printf(" Frame queued to socket receive queue.\n");
                /* Frame queued to socket receive queue */
                len = recv(g_skfd, buf, sizeof(buf), MSG_DONTWAIT);
                if (len <= 0)
                    break;
                nlh_rx = buf;
            }
            else
                /* No more messages to process, continue polling */
                break;

            if(DEBUG_LEVEL == 3)
                printf("kernel: %s\n", NLMSG_DATA(nlh_rx));



            /* Read message from kernel */
            {
                if(DEBUG_LEVEL == 3)
                {
                    printf("Received from kernel[1]: %s\n", NLMSG_DATA(nlh_rx));
                }



                char *ar1;
                size_t ar_len;
                //int i;
                const char *s = NLMSG_DATA(nlh_rx);
                const char *PATTERN1_1 = "\"survey->time_busy:\":";
                const char *PATTERN2_1 = ",";

                func1(&ar1, &ar_len, s, PATTERN1_1, PATTERN2_1);

                if(DEBUG_LEVEL == 3)
                {
                    printf("from the caller fucntion:%s\n", ar1);
                }
                survey_time_busy_4 = atoi(ar1);
                free( ar1 );

                char *ar2;
                size_t ar_len2;
                //int i;
                const char *PATTERN1_2 = "\"survey->time\":";
                const char *PATTERN2_2 = ",";

                func1(&ar2, &ar_len2, s, PATTERN1_2, PATTERN2_2);

                if(DEBUG_LEVEL == 3)
                {
                    printf("from the caller fucntion:%s\n", ar2);
                }
                survey_time_4 = atoi(ar2);

                free( ar2 );




                char *ar3;
                size_t ar_len3;
                const char *PATTERN1_3 = "Timestamp2\":";
                const char *PATTERN2_3 = "}";

                func1(&ar3, &ar_len3, s, PATTERN1_3, PATTERN2_3);

                char *ptr;
                double ret;

                timestamp_4_2 = strtod(ar3, &ptr);

                free( ar3 );


                char *ar4;
                size_t ar_len4;
                const char *PATTERN1_4 = "Timestamp1\":";
                const char *PATTERN2_4 = ",";

                func1(&ar4, &ar_len4, s, PATTERN1_4, PATTERN2_4);

                char *ptr4;
                double ret4;

                timestamp_4_1 = strtod(ar4, &ptr4);

                free( ar4 );




            }



            float channel_utlization_raw = 2.3;
            channel_utlization_raw = (float)(survey_time_busy_4 - survey_time_busy_1) / (survey_time_4 - survey_time_1);
            if(DEBUG_LEVEL == 3)
            {
                printf("Value of survey_time_busy2 = %d\n", survey_time_busy_4 - survey_time_busy_1);
                printf("Value of survey_time2 = %d\n", survey_time_4 - survey_time_1);
                printf("Value of channel_utlization_raw2 = %f\n", channel_utlization_raw);
                printf("Value of Timestamp1 from kernel @ data collection in [seconds] = %lf\n\n", timestamp_4_1);
                printf("Delta of Timestamp1 from kernel @ data collection in [seconds] = %lf\n\n", timestamp_4_1 - timestamp_1_1);
                printf("Value of Timestamp2 from kernel @ data collection in [seconds] = %lf\n\n", timestamp_4_2);
                printf("Delta of Timestamp2 from kernel @ data collection in [seconds] = %lf\n\n", timestamp_4_2 - timestamp_1_2);
                printf("Delta of Timestamp2 and Timestamp1 in [seconds] = %lf\n\n", timestamp_4_2 - timestamp_4_1);
                printf("for_avg_deltat2_t2 in [seconds] = %lf\n\n", for_avg_deltat2_t2);
            }
            if(timestamp_4_2 - timestamp_4_1 > 0)
                for_avg_deltat2_t2 = for_avg_deltat2_t2 + (timestamp_4_2 - timestamp_4_1);



            {



                survey_time_busy_1 = survey_time_busy_4;
                survey_time_1 = survey_time_4;

                timestamp_1_2 = timestamp_4_2;
                timestamp_1_1 = timestamp_4_1;


release:
                /* Release frame back to the kernel */
                hdr_rx->nm_status = NL_MMAP_STATUS_UNUSED;

                /* Advance frame offset to next frame */
                g_rx_fr_off = (g_rx_fr_off + req.nm_frame_size) % g_ring_size;
            }
            kernel_interations++;
            // printf("Value of kernel_interations = %d\n\n", kernel_interations);

        }

    }
    long int end_t = GetTimeStamp();
    printf("time in micro final : %lu \n", end_t - start_t);
    printf("time in mili  final : %lu \n", (end_t - start_t) / 1000);
    printf("time in micro per interation  final : %lu \n", ( end_t - start_t) / ( kernel_interations * ITERATIONS));
    printf("kernel_interations  final : %lu \n", kernel_interations);
    printf("time in mili per iteration  final : %d \n", ((end_t - start_t) / 1000) / kernel_interations);
    printf("average deltat2_t1 in seconds : %lf \n", (for_avg_deltat2_t2 / kernel_interations));
    printf("for log: %lu, %lu, %lu, %d, %s\n", start_t, end_t, ( end_t - start_t) / ( kernel_interations * ITERATIONS), ITERATIONS, buffer);

    close(g_skfd);
    sleep(2);
    return 0;
}
