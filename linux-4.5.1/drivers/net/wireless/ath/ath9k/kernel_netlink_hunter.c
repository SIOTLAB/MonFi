#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/netlink.h>

// defination for the netlin components
#define MAX_MSGSIZE 125
#define NL_FR_SZ    16384
#define TDB_NLMSG_MAXSZ     (NL_FR_SZ / 2 - NLMSG_HDRLEN - MAX_MSGSIZE \
                 - MAX_MSGSIZE)

// using port 30 for mmapped-netlink
#define NETLINK_TEST 30

// this the main struct which we borrow from the atheror debug file and used to collect the measurements
struct ath_softc *sc;

//debug_level global variable used for debugging purposes. All the printk statements would be directed to the kernel log file and impose and overhead.
// as of now, we only have two debug modes: debug_level == 0 - prints none. debug_level >0 - prints all
// the debug_level can be modified during runtime from the Controller. Check for further info in README file.
int debug_level = 9;

char temp_message[1024];
struct sock *nl_sk = NULL;


// this function will called by the debug.c->open_file_regdump function -via temp(inode);
// used for retrieving the ath_softc data structure from the native atheros driver.
void temp(struct inode *inode)
{
    sc = inode->i_private;
    if (debug_level >= 1)
    {
        printk(KERN_INFO "this is the temp function\n");
        printk(KERN_DEBUG "from temp: %d", REG_READ(sc->sc_ah, AR_CCCNT));
        printk("kernel start recv from user: \n");
    }

}

//this function helps us in collecting AU. the two important register values it reads is AR_CCCNT and AR_RCCNT

void update_AU(struct ath_common *common)
{
    u32 cycles, busy, rx, tx;
    void *ah = common->ah;

    /* freeze */
    REG_WRITE(ah, AR_MIBC, AR_MIBC_FMC);            //lock the registers while reading

    /* read */
    cycles = REG_READ(ah, AR_CCCNT);
    busy = REG_READ(ah, AR_RCCNT);
    rx = REG_READ(ah, AR_RFCNT);
    tx = REG_READ(ah, AR_TFCNT);

    /* clear */
    REG_WRITE(ah, AR_CCCNT, 0);                     //initializes all the register values for next iteration
    REG_WRITE(ah, AR_RFCNT, 0);
    REG_WRITE(ah, AR_RCCNT, 0);
    REG_WRITE(ah, AR_TFCNT, 0);

    /* unfreeze */
    REG_WRITE(ah, AR_MIBC, 0);

    /* update all cycle counters here */
    common->cc_survey.cycles += cycles;             //update the struct which stores the surveyed information. This is where ethtool reads information from 
    common->cc_survey.rx_busy += busy;
    common->cc_survey.rx_frame += rx;
    common->cc_survey.tx_frame += tx;
}

// return the wallclock time in microsecond. Used for timekeeping and determining when to sleep
static long getMicrotime(void)
{
    struct timeval currentTime;
    do_gettimeofday(&currentTime);
    return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}



static int tdb_if_info(struct sk_buff *skb, struct netlink_callback *cb)
{
    struct nlmsghdr *nlh;

    nlh = nlmsg_put(skb, NETLINK_CB(cb->skb).portid, cb->nlh->nlmsg_seq,
                    cb->nlh->nlmsg_type, TDB_NLMSG_MAXSZ, 0);
    if (!nlh)
        return -EMSGSIZE;

    nlmsg_data(nlh);

    /* Fill in response. */
    char kmsg[] = "hello users!!!";
    int slen = strlen(temp_message);
    memcpy(nlmsg_data(nlh), temp_message, slen);
    return 0;
}

// whenever we receive a message from the user, the callback function will be invoked for processing the message
static int tdb_if_proc_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    char kmsg[] = "hello users!!!"; //filler message
    char *m = NULL;
    m = nlmsg_data(nlh);    //Returns a pointer to the payload associated with the passed nlmsghdr.

    if (debug_level >= 1)
    {
        if(m)                                       //just checking if message from the Controller looks correct.
        {
            printk("kernel recv from user: %s\n", m);
        }
    }
    long abs_time = getMicrotime();                 // keeps the log of the absolute time.


    {
        int total_kernel_interations = 5;           //total measurements requested by the Controller
        int current_kernel_interations = 0;         //keep log of the current iteration of collecting the measurements.

        int count = 0;                              //keeps track of number of arguments requested by the Controller

        int res;
        int var, rett;
        // rett = kstrtoint(msg, 10, &sample_duration);
        // printk(KERN_DEBUG "ret: %d\n", rett);
        // printk(KERN_DEBUG "var: %d\n", sample_duration);
        int sample_duration = 0;                    // Will store the IMI requested by the Controller
        int prev_sample_duration = 0;               // Will store the previous IMI
        int one, two, three, four, five, six;       // for the register addresses requested by the Controller. Initialized from the message sent by the Controller.
        int one_reg_val, two_reg_val, three_reg_val, four_reg_val, five_reg_val;    //will actually store the register values on those addresses. Will be used every iteration

        char *string, *found;
        char *str1 = kmalloc(sizeof(char) * 254, GFP_KERNEL);
        strcpy(str1, m);                            // m contains the message we received, copying into str1 for extarcting parameters from the request sent by the Controller

        if (debug_level >= 1)
        {
            printk(KERN_DEBUG "Original string: '%s'\n", str1);
        }
        while( (found = strsep(&str1, " ")) != NULL )//extracting space separated parameters from the request-string sent by the Controller. For each fine, count variable would be incremented
        {
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG  "%s\n", found);
            }
            if(count == 0 )
            {
                rett = kstrtoint(found, 10, &sample_duration);      //convert string to int from within the kernel module.
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "sample_duration: %d\n", sample_duration);    //IMI requested by the Controller
                    if (sample_duration == 0)
                        sample_duration = prev_sample_duration;
                    printk(KERN_DEBUG "final sample_duration: %d\n", sample_duration);
                }
            }

            if(count == 1 )
            {
                rett = kstrtoint(found, 10, &total_kernel_interations);         //total number of iterations requested by the Controller
                {
                    if (debug_level >= 1)
                        printk(KERN_DEBUG "var: %d\n", total_kernel_interations);
                }
            }

            if(count == 2 )
            {
                rett = kstrtoint(found, 0, &one);                               //one two three for and five are the register addresses.
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one: %x\n", one);
                }

            }
            if(count == 3 )
            {
                rett = kstrtoint(found, 0, &two);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "two: %x\n", two);
                }
            }
            if(count == 4 )
            {
                rett = kstrtoint(found, 0, &three);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "three: %x\n", three);
                }
            }
            if(count == 5 )
            {
                rett = kstrtoint(found, 0, &four);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "four: %d\n", four);
                }
            }
            if(count == 6 )
            {
                rett = kstrtoint(found, 0, &five);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "five: %d\n", five);
                }
            }
            if(count == 7 )
            {
                rett = kstrtoint(found, 0, &six);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "six: %d\n", six);
                }
                debug_level = six;                                  //debug_level is modified by six, i.e last parameter of the Controller request
            }




            count = count + 1;
        }
        mutex_lock(&sc->mutex);                                     //grabbing the mutex lock before tempering the ath data structures, can use spin_lock_bh and spin_unlock_bh as well

        if (debug_level >= 1)
        {
            printk(KERN_DEBUG "total_kernel_interations : %d\n", total_kernel_interations);
        }

        // END: initialization utilizing the request sent by the Controller
        // BEGIN: collecting measurements
        while (current_kernel_interations < total_kernel_interations)   //while until total_kernel_iterations are collected
        {
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "current_kernel_interations : %d\n", current_kernel_interations);
            }
            current_kernel_interations = current_kernel_interations + 1;    //increment current_kernel_iterations
            int k = 0;
            unsigned long flags;
            // extra checks in for exclusitivity on the CPU core while running the measurements.
            preempt_disable();                                              //we disable preemption on our CPU
            raw_local_irq_save(flags);                                      //we disable hard interrupts on our CPU, at this stage we exclusively own the CPU

            // time keeping - nanosecond level
            struct timespec ts1;
            getnstimeofday(&ts1);
            char *temp1;
            temp1 = kmalloc(50 * sizeof(char), GFP_KERNEL);
            sprintf(temp1, "%ld.%ld seconds\n", ts1.tv_sec, ts1.tv_nsec);
            
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "Timestamp [1]: %s\n", temp1);
            }

            //start reading the values at the register addresses specified by the Contoller and store it in variables as one_reg_val, two_reg_val ...
            // count would be utilized from the outside the while loop. It helps us determine the number of addresses specified by the user Contoller request
            if (count == 3)
            {
                one_reg_val = REG_READ(sc->sc_ah, one);                     //REG_READ function eventually calls ->  ath9k_ioread32 ->  ioread32. REG_READ uses spin_lock_irqsave
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
                }

            }
            else if (count == 4)
            {
                one_reg_val = REG_READ(sc->sc_ah, one);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
                }
                two_reg_val = REG_READ(sc->sc_ah, two);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
                }

            }
            else if (count == 5)
            {
                one_reg_val = REG_READ(sc->sc_ah, one);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
                }
                two_reg_val = REG_READ(sc->sc_ah, two);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
                }
                three_reg_val = REG_READ(sc->sc_ah, three);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", three_reg_val);
                }

            }
            else if (count == 6)
            {
                one_reg_val = REG_READ(sc->sc_ah, one);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
                }
                two_reg_val = REG_READ(sc->sc_ah, two);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
                }
                three_reg_val = REG_READ(sc->sc_ah, three);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", three_reg_val);
                }

                four_reg_val = REG_READ(sc->sc_ah, four);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "one_reg_val: %d\n", four_reg_val);
                }

            }


            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "the value of count : %d\n", count);
            }
            // END reading the registers
            // BEGIN obtaining channel utilization
            {






                char *outStr =  kmalloc (sizeof (char) * 500, GFP_KERNEL);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "[ath9k][link][ath_update_survey_stats]: \n");
                }
                struct ath_hw *ah = sc->sc_ah;                                      //extracting ath_hw from ath_softc
                struct ath_common *common = ath9k_hw_common(ah);            
                int pos = ah->curchan - &ah->channels[0];                           //obtainin the current channel, on which we need the AU
                struct survey_info *survey = &sc->survey[pos];
                struct ath_cycle_counters *cc = &common->cc_survey;
                unsigned int div = common->clockrate; //registers get the value  in clock cycle, common->clockrate gets the clockrate of operation. we can use this to get the AU inforamation in milliseconds or microseconds. 
                // we will be using the raw register values and convert them into microseconds in user space to reduce the overhead of conversion. div would be used for other tools like ethtool
                int ret = 0;                                                        

                //Time keeping for AU collection
                struct timespec ts2;
                getnstimeofday(&ts2);
                char *temp2;
                temp2 = kmalloc(50 * sizeof(char), GFP_KERNEL);
                sprintf(temp2, "%ld.%ld seconds\n", ts2.tv_sec, ts2.tv_nsec);
                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "Timestamp [2]: %s\n", temp2);
                }


                if (!ah->curchan)                                   //return -1 no channel found.
                    return -1;

                if(count != 2)
                {
                    if (ah->power_mode == ATH9K_PM_AWAKE)           //check the power state of the NIC. 
                        update_AU(common);                          // if awake, update the airtime utilization
                    raw_local_irq_restore(flags);                   //we enable hard interrupts on our CPU as we are done utilizing the atheros struct
                    preempt_enable();                               //we enable preemption

                    if (cc->cycles > 0)
                    {

                        survey->filled |= SURVEY_INFO_TIME |
                                          SURVEY_INFO_TIME_BUSY |
                                          SURVEY_INFO_TIME_RX |
                                          SURVEY_INFO_TIME_TX;
                        survey->time += cc->cycles / div;           //the survey struct is used by other utilities, such as ethtool. Hence, update those. Comment this part if you require higher efficiency.
                        survey->time_busy += cc->rx_busy / div;
                        survey->time_rx += cc->rx_frame / div;
                        survey->time_tx += cc->tx_frame / div;

                        int intvar = 100;

                    }
                }
                mutex_unlock(&sc->mutex);                           //release the mutex_lock
                
                // utilizing the count variable, we prepare the message that we will send to the user space ... outStr
                if(count == 2)
                {
                    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 100, 100, 100, 100, 100, 100, 100, 100, one, ts2.tv_sec, ts2.tv_nsec);

                }

                if(count == 3)
                {
                    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx, one, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
                }
                else if(count == 4)
                {
                    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx, one, two, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
                }
                else if(count == 5)
                {
                    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx, one, two, three, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
                }
                else if(count == 6)
                {
                    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"four\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx, one, two, three, four, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
                }
                else if(count == 7)
                {
                    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"four\":%d, \"five\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx, one, two, three, four, five, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
                }
                else
                {
                    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"four\":%d, \"five\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx, one, two, three, four, five, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
                }




                if (debug_level >= 1)
                {
                    printk(KERN_DEBUG "String to be sent back: %s\n", outStr);
                }

                memset(cc, 0, sizeof(*cc));
                ath_update_survey_nf(sc, pos);

                if (debug_level >= 1)
                {
                    printk(KERN_INFO "netlink_test: Send %s\n", outStr);
                }
                strcpy(temp_message, outStr);


                // sending the message contianing the measurements with AU details to the user space.
                if (debug_level >= 1)
                    printk(KERN_INFO "temp_message: Send %s\n", temp_message);

                struct netlink_dump_control c =
                {
                    .dump = tdb_if_info,
                    //.data = m,
                    .min_dump_alloc = NL_FR_SZ / 2,
                };
                netlink_dump_start(nl_sk, skb, nlh, &c);

                //housekeeping
                kfree(outStr);
                k = k + 1;


                //end time
                char *temp21;
                temp21 = kmalloc(50 * sizeof(char), GFP_KERNEL);
                struct timespec ts21;
                getnstimeofday(&ts21);
                sprintf(temp21, "%ld.%ld seconds\n", ts21.tv_sec, ts21.tv_nsec);

                //determing how much to sleep according to the measurement colection time and IMI
                // abs_time keeps track of when the measurements are expected. i.e., start time + sample duration, every iteration
                // if the current time is less then abs_time, then sleep for sleep_duration = abs_time - current time
                abs_time = abs_time + sample_duration;
                int sleep_duration;
                int jj = 9;
                if (jj == 9)
                {

                    long now_time = getMicrotime() ;
                    if ((abs_time - now_time) > 0)
                        sleep_duration = abs_time - now_time;
                    else
                        sleep_duration = 0;
                    if (debug_level >= 1)
                        printk(KERN_INFO "abstime:%ld, now time:%ld, delta:%d, Sleeping for:%d\n", abs_time, now_time, abs_time - now_time, sleep_duration);
                }
                prev_sample_duration = sample_duration;

                if(sleep_duration > 0)
                {
                    if (sleep_duration < (1000000))
                        usleep_range(sleep_duration, sleep_duration + 1);       //most efficient way of sleeping in a kernel.
                    else if (sleep_duration >= (1000000))
                        msleep(sleep_duration / 1000);
                    else
                    {
                        if (debug_level >= 1)
                        {
                            printk(KERN_INFO "Unable to Sleep %s\n");
                        }
                    }
                }

            }
        }
    }



    return 1 ;
}

static void tdb_if_rcv(struct sk_buff *skb)
{
    /* TODO remove the mutex for concurrent user-space updates. */
    struct nlmsghdr *nlh = NULL;
    nlh = nlmsg_hdr(skb);
    tdb_if_proc_msg(skb, nlh);
}

struct netlink_kernel_cfg cfg =
{
    .input = tdb_if_rcv, /* set recv callback */
};

int myinit_module(void)
{
    printk("my netlink in\n");
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    if(nl_sk == NULL)
        printk("kernel_create error\n");
    return 0;
}

void mycleanup_module(void)
{
    printk("my netlink out!\n");
    sock_release(nl_sk->sk_socket);
    netlink_kernel_release(nl_sk);
}


// we do not need to init end exit the module as that would taken care by the ath9k debug.c module. Remember that we are importing this file from the debug.c file
// module_init(myinit_module);
// module_exit(mycleanup_module);
