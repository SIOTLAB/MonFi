/* kernel_netlink.c ---
 *
 * Filename: kernel_netlink.c
 * Description:
 * Author: Andrey Andruschenko
 * Maintainer:
 * Created: Чт фев  9 15:42:17 2017 (+0300)
 * Version:
 * Package-Requires: ()
 * Last-Updated:
 *           By:
 *     Update #: 196
 * URL:
 * Doc URL: https://fpbrain.blogspot.ru/2017/02/mmaped-netlink-in-linux-kernel-zero.html
 * Keywords: linux, netlink, mmap
 * Compatibility:
 *
 */

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#include "nl_msg.h"

struct sock *nl_sk = NULL;
struct ath_softc *sc;
int debug_level = 9;

void temp1(struct inode *inode)
{
    sc = inode->i_private;
    if (debug_level >= 1)
    {
        printk(KERN_INFO "this is the temp function\n");
        printk(KERN_DEBUG "from temp: %d", REG_READ(sc->sc_ah, AR_CCCNT));
    }

}



static char *print_out_m_type(m_type_t message) {
    static char out[64];
    char *ptr = NULL;
    size_t i = 0;
    int len = 0;
    struct mtypes {
        m_type_t type;
        char *name;
    } m_array[] = {{MSG_OK, "MSG_OK"},
                   {MSG_PING, "MSG_PING"},
                   {MSG_PONG, "MSG_PONG"},
                   {MSG_DATA, "MSG_DATA"}};

    ptr = out;

    for (i = 0; i < sizeof(m_array) / sizeof(m_array[0]); i++) {
        if (message & m_array[i].type) {
            len = sprintf(ptr, " %s |", m_array[i].name);
            ptr = (char *)((size_t)ptr + (size_t)len);
        }
    }

    sprintf(((char *)(void *) ptr - 1), "(%d)", message);

    return out;
}


//  struct netlink_dump_control c = {
//         .dump = hello_nl_send_msg, .data = msg, .min_dump_alloc = NL_FR_SZ / 2,
//     };
    
//     netlink_dump_start(nl_sk, skb, nlh, &c);


//    msg = nlmsg_data(nlh);
//     usr_message = (char *)((void *)msg + sizeof(us_nl_msg_t));

// char *usr_message;
// static int hello_nl_send_msg(struct sk_buff *skb, struct netlink_callback *cb) {
//     struct nlmsghdr *nlh;
//     us_nl_msg_t *resp, *req = cb->data;
//     int msg_len = 0;
//     char *resp_msg = NULL;

//     if (cb->data != NULL) msg_len = req->len;

//     if (!(nlh = nlmsg_put(skb, NETLINK_CB(cb->skb).portid, cb->nlh->nlmsg_seq, cb->nlh->nlmsg_type,
//                           sizeof(us_nl_msg_t) + msg_len, 0)))
//         return -EMSGSIZE;

//     resp = (us_nl_msg_t *)nlmsg_data(nlh);

//     if (req->type & MSG_DATA) {
//         resp_msg = (char *)((void *)req + sizeof(us_nl_msg_t));
//         resp->type = MSG_OK | MSG_DATA;
//         resp->len = msg_len;
//         memcpy((void *)((void *)resp + sizeof(us_nl_msg_t)), resp_msg, msg_len);
//     }

static int hello_nl_send_msg(struct sk_buff *skb, struct netlink_callback *cb) {
    printk(KERN_ERR "Entered hello_nl_send_msg\n");
    struct nlmsghdr *nlh;
    us_nl_msg_t *resp, *req = cb->data;
    int msg_len = 0;
    char *resp_msg = NULL;
    char *usr_message;
    printk(KERN_ERR "Entered hello_nl_send_msg 1\n");
    if (cb->data != NULL) msg_len = strlen((char *)((void *)req + sizeof(us_nl_msg_t)));

    if (!(nlh = nlmsg_put(skb, NETLINK_CB(cb->skb).portid, cb->nlh->nlmsg_seq, cb->nlh->nlmsg_type,
                          sizeof(us_nl_msg_t) + msg_len, 0)))
        return -EMSGSIZE;

    printk(KERN_ERR "Entered hello_nl_send_msg 2\n");
    resp = (us_nl_msg_t *)nlmsg_data(nlh);
    // usr_message = (char *)((void *)resp + sizeof(us_nl_msg_t));
    // printk(KERN_INFO " usr_message ...... message %.*s\n", usr_message);

    printk(KERN_ERR "Entered hello_nl_send_msg 3 \n");
    if (req->type & MSG_DATA) {
        resp_msg = (char *)((void *)req + sizeof(us_nl_msg_t));
        resp->type = MSG_OK | MSG_DATA;
        resp->len = strlen(resp_msg);
        // printk(KERN_INFO " resp_msg ...... message %.*s\n", resp_msg);
        memcpy((void *)((void *)resp + sizeof(us_nl_msg_t)), resp_msg, strlen(resp_msg));
    }
    if (cb->data != NULL) msg_len = strlen(resp_msg);


    printk(KERN_ERR "Entered hello_nl_send_msg 4\n");
    if (req->type & MSG_PING) resp->type |= (MSG_OK | MSG_PONG);

    printk(KERN_INFO "Sending %lu/%lu bytes to userspace.\n", resp->len,
           sizeof(us_nl_msg_t) + resp->len);

    return 0;
}





static int hello_nl_recv_msg(struct sk_buff *skb, struct nlmsghdr *nlh) {
    us_nl_msg_t *msg;
    char *usr_message;
    char *temp_message = kmalloc (sizeof (char) * 500, GFP_KERNEL);;
    if (nlh->nlmsg_len < sizeof(*nlh) + sizeof(us_nl_msg_t)) {
        printk(KERN_ALERT "Message to short %d", nlh->nlmsg_len);
        return -EINVAL;
    }

    msg = nlmsg_data(nlh);
    usr_message = (char *)((void *)msg + sizeof(us_nl_msg_t));

    printk(KERN_INFO "From [%u] msg type %s , payload len: %d, message %.*s\n", nlh->nlmsg_pid,
           print_out_m_type(msg->type), (int)msg->len, (int)msg->len, usr_message);

//------------------------------------------
{
    int total_kernel_interations = 5;
    int current_kernel_interations = 0;

    int count = 0;

    // struct sk_buff *skb_out;
    // struct nlmsghdr *nlh;
    // int msg_size;
    // char *msg;
    // int pid;
    int res;

    // nlh = (struct nlmsghdr *)skb->data;
    // pid = nlh->nlmsg_pid;  pid of sending process 
    // msg = (char *)nlmsg_data(nlh);
    // msg_size = strlen(msg);



    //[START]parsing the received string for the registers and and fetching the register status.
    int var, rett;
    // rett = kstrtoint(msg, 10, &sample_duration);
    // printk(KERN_DEBUG "ret: %d\n", rett);
    // printk(KERN_DEBUG "var: %d\n", sample_duration);
    int sample_duration = 0;

    int one, two, three, four, five,six;
    int one_reg_val, two_reg_val, three_reg_val, four_reg_val, five_reg_val;

    char *string, *found;

    //string = strdup("Hello there, peasants!");
    char *str1 = kmalloc(sizeof(char) * 254, GFP_KERNEL);
    strcpy(str1, usr_message);
    if (debug_level >= 1)
    {
        printk(KERN_DEBUG "Original string: '%s'\n", str1);
    }
    while( (found = strsep(&str1, " ")) != NULL )
    {
        if (debug_level >= 1)
        {
            printk(KERN_DEBUG  "%s\n", found);
        }
        if(count == 0 )
        {
            rett = kstrtoint(found, 10, &sample_duration);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "var: %d\n", sample_duration);
            }
        }

        if(count == 1 )
        {
            rett = kstrtoint(found, 10, &total_kernel_interations);
            {
                 if (debug_level >= 1)
                printk(KERN_DEBUG "var: %d\n", total_kernel_interations);
            }
        }

        if(count == 2 )
        {
            rett = kstrtoint(found, 0, &one);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one: %x\n", one);
            }
            // one_reg_val = REG_READ(sc->sc_ah, one);
            // printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);

        }
        if(count == 3 )
        {
            rett = kstrtoint(found, 0, &two);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "two: %x\n", two);
            }
            // two_reg_val = REG_READ(sc->sc_ah, two);
            // printk(KERN_DEBUG "two_reg_val: %d\n", two_reg_val);
        }
        if(count == 4 )
        {
            rett = kstrtoint(found, 0, &three);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "three: %x\n", three);
            }
            // three_reg_val = REG_READ(sc->sc_ah, three);
            // printk(KERN_DEBUG "three_reg_val: %d\n", three_reg_val);
        }
        if(count == 5 )
        {
            rett = kstrtoint(found, 0, &four);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "four: %d\n", four);
            }
            // four_reg_val = REG_READ(sc->sc_ah, four);
            // printk(KERN_DEBUG "four_reg_val: %d\n", four_reg_val);
        }
        if(count == 6 )
        {
            rett = kstrtoint(found, 0, &five);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "five: %d\n", five);
            }
            // five_reg_val = REG_READ(sc->sc_ah, five);
            // printk(KERN_DEBUG "five_reg_val: %d\n", five_reg_val);
        }
        if(count == 7 )
        {
            rett = kstrtoint(found, 0, &six);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "six: %d\n", six);
            }
            debug_level = six;
            // five_reg_val = REG_READ(sc->sc_ah, five);
            // printk(KERN_DEBUG "five_reg_val: %d\n", five_reg_val);
        }




        count = count + 1;
    }
  //    mutex_lock(&sc->mutex);
        // ath9k_set_txpower(sc, NULL);
        // mutex_unlock(&sc->mutex);

    while (current_kernel_interations < total_kernel_interations)
    {
        current_kernel_interations = current_kernel_interations + 1;
        int k = 0;
        unsigned long flags;
        preempt_disable(); /*we disable preemption on our CPU*/
        raw_local_irq_save(flags); /*we disable hard interrupts on our CPU*/    /*at this stage we exclusively own the CPU*/

        struct timespec ts1;
        // char *temp;
        // temp=kmalloc(50*sizeof(char),GFP_KERNEL);
        getnstimeofday(&ts1);
        // sprintf(temp,"%ld seconds \n%ld nanoseconds\n",ts.tv_sec, ts.tv_nsec);
        // printk(KERN_DEBUG "Timestamp: %s\n", temp);

        char *temp1;
        temp1 = kmalloc(50 * sizeof(char), GFP_KERNEL);
        // getnstimeofday(&ts);
        sprintf(temp1, "%ld.%ld seconds\n", ts1.tv_sec, ts1.tv_nsec);
        if (debug_level >= 1)
        {
            printk(KERN_DEBUG "Timestamp [1]: %s\n", temp1);
        }

        if (count == 3)
        {
            one_reg_val = REG_READ(sc->sc_ah, one);
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

        //[END]parsing the received string for the registers and and fetching the register status.




        // while(k < 1)
        {





            //printk(KERN_DEBUG "from netlink_test_recv_msg: %d", REG_READ(sc->sc_ah, AR_CCCNT));
            //get_stats();
            //char buff_to_send[500];
            //char *buff_to_send = get_stats_ath_update_survey_stats();

            char *outStr =  kmalloc (sizeof (char) * 500, GFP_KERNEL);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "[ath9k][link][ath_update_survey_stats]: \n");
            }
            struct ath_hw *ah = sc->sc_ah;
            struct ath_common *common = ath9k_hw_common(ah);
            int pos = ah->curchan - &ah->channels[0];
            struct survey_info *survey = &sc->survey[pos];
            struct ath_cycle_counters *cc = &common->cc_survey;
            unsigned int div = common->clockrate;
            int ret = 0;


            //    printk(KERN_DEBUG "NO. of Beacons %d", REG_READ(sc->sc_ah, one));
            // printk(KERN_DEBUG "NAV value %d", REG_READ(sc->sc_ah, two));
            // printk(KERN_DEBUG "NAV value %d", REG_READ(sc->sc_ah, three));
            struct timespec ts2;
            // char *temp;
            // temp=kmalloc(50*sizeof(char),GFP_KERNEL);
            getnstimeofday(&ts2);
            // sprintf(temp,"%ld seconds \n%ld nanoseconds\n",ts.tv_sec, ts.tv_nsec);
            // printk(KERN_DEBUG "Timestamp: %s\n", temp);

            char *temp2;
            temp2 = kmalloc(50 * sizeof(char), GFP_KERNEL);
            // getnstimeofday(&ts);
            sprintf(temp2, "%ld.%ld seconds\n", ts2.tv_sec, ts2.tv_nsec);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "Timestamp [2]: %s\n", temp2);
            }


            if (!ah->curchan)
                return -1;

            if(count != 2)
            {
                if (ah->power_mode == ATH9K_PM_AWAKE)
                    ath_hw_cycle_counters_update(common);
                raw_local_irq_restore(flags); /*we enable hard interrupts on our CPU*/
                preempt_enable();/*we enable preemption*/

                if (cc->cycles > 0)
                {

                    survey->filled |= SURVEY_INFO_TIME |
                                      SURVEY_INFO_TIME_BUSY |
                                      SURVEY_INFO_TIME_RX |
                                      SURVEY_INFO_TIME_TX;
                    survey->time += cc->cycles / div;
                    survey->time_busy += cc->rx_busy / div;
                    survey->time_rx += cc->rx_frame / div;
                    survey->time_tx += cc->tx_frame / div;
                    /*
                    printk(KERN_DEBUG "cc->cycles: %lu\n", cc->cycles);
                    printk(KERN_DEBUG "cc->rx_busy: %lu\n", cc->rx_busy);
                    printk(KERN_DEBUG "cc->rx_frame: %lu\n", cc->rx_frame);
                    printk(KERN_DEBUG "cc->tx_frame: %lu\n", cc->tx_frame);
                    printk(KERN_DEBUG "survey->time: %lu\n", survey->time);
                    printk(KERN_DEBUG "survey->time_busy: %lu\n", survey->time_busy);
                    printk(KERN_DEBUG "survey->time_rx: %lu\n", survey->time_rx);
                    printk(KERN_DEBUG "survey->time_tx: %lu\n", survey->time_tx);
                    printk(KERN_DEBUG "common->clockrate: %lu\n", common->clockrate);
                    */

                    int intvar = 100;
                    //char str[16];
                    // '{"name":"John", "age":31, "city":"New York"}'
                    //printk(KERN_DEBUG "sizeof(outStr): %d\n", sizeof(outStr));

                }
            }
            // if (debug_level >= 1)
            // {
            //     printk(KERN_INFO "[1]netlink_test: Received from pid %d: %s\n", pid, msg);
            // }
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




            //snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "String to be sent back in json: %s\n", outStr);
            }
            //if (cc->cycles < div)
            //  return -1;

            if (cc->cycles > 0)
                ret = cc->rx_busy * 100 / cc->cycles;

            memset(cc, 0, sizeof(*cc));

            ath_update_survey_nf(sc, pos);// move this before preparing the string
            //return outStr;



            // // create reply
            // skb_out = nlmsg_new(strlen(outStr), 0);
            // if (!skb_out)
            // {
            //     if (debug_level >= 1)
            //     {
            //         printk(KERN_ERR "netlink_test: Failed to allocate new skb\n");
            //     }
            //     return;
            // }




            // // put received message into reply
            // nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, strlen(outStr), 0);
            // NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
            // strncpy(nlmsg_data(nlh), outStr, strlen(outStr));

            if (debug_level >= 1)
            {
                printk(KERN_INFO "netlink_test: Send %s\n", outStr);
            }
            // res = nlmsg_unicast(nl_sock, skb_out, pid);
            // if (res < 0)
                // printk(KERN_INFO "netlink_test: Error while sending skb to user, err code %d\n", res);
            // if(count == 2)
            // {
            //    snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d}", cc->cycles, cc->rx_busy);        

            // }
            strcpy(temp_message, outStr);
            kfree(outStr);
            k = k + 1;
            if (debug_level >= 1)
            {
                printk(KERN_INFO "Sleeping %s\n");
            }
            udelay(sample_duration);
        }
    }
}

//------------------------------------------
    char *outStr1 =
        "1 2 0x8098 0x8098 0x5555 0x8098 0x5555";
         printk(KERN_INFO "temp_message: Send %s\n", temp_message);
          printk(KERN_INFO "[hello_nl_recv_msg].................20\n");
    memcpy((void *)((void *)msg + sizeof(temp_message)), temp_message, strlen(temp_message));
    printk(KERN_INFO "[hello_nl_recv_msg].................21\n");
    struct netlink_dump_control c = {
        .dump = hello_nl_send_msg, .data = msg, .min_dump_alloc = NL_FR_SZ / 2,
    };
    printk(KERN_INFO "[hello_nl_recv_msg].................22\n");
   netlink_dump_start(nl_sk, skb, nlh, &c);
   printk(KERN_INFO "[hello_nl_recv_msg].................23\n");
   kfree(temp_message);
   printk(KERN_INFO "[hello_nl_recv_msg].................24\n");
    return 1;
    // return netlink_dump_start(nl_sk, skb, nlh, &c);
}

static void if_rcv(struct sk_buff *skb) {
    printk(KERN_INFO "[if_rcv].................1\n");
    struct nlmsghdr *nlh;
     printk(KERN_INFO "[if_rcv].................2\n");
    nlh = nlmsg_hdr(skb);
     printk(KERN_INFO "[if_rcv].................3\n");
    netlink_rcv_skb(skb, &hello_nl_recv_msg);
     printk(KERN_INFO "[if_rcv].................4\n");
}

static int kern_netlink_init(void) {
    struct netlink_kernel_cfg cfg = {
        .input = if_rcv,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USERSOCK, &cfg);

    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    } else
        printk(KERN_INFO "Netlink socket created.\n");

    return 0;
}

static void kern_netlink_exit(void) { netlink_kernel_release(nl_sk); }

//module_init(kern_netlink_init);
//module_exit(kern_netlink_exit);

MODULE_LICENSE("GPL");

/* kernel_netlink.c ends here */
