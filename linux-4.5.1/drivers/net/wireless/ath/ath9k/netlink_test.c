#include <net/sock.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>



#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/export.h>
#include <asm/unaligned.h>
#include <linux/delay.h>


// #include <stdlib.h>
#include "ath9k.h"

#define NETLINK_TEST2 17
#define REGDUMP_LINE_SIZE	2000
//#include "nl1.h"



int debug_level2 = 9;
// default debug_level2 is 5
// if debug_level2 0, then print nothing
// most important => if debug_level2 > x, where x ranges from 1...9, 1 is least important, and 9 is the most important



struct sock *nl_sock = NULL;
struct ath_softc *sc2;

void temp2(struct seq_file *file)
{
    struct ieee80211_hw *hw = dev_get_drvdata(file->private);
    sc2 = hw->priv;

    // sc = inode->i_private;
    if (debug_level2 >= 1)
    {
        printk(KERN_INFO "this is the temp function\n");
        printk(KERN_DEBUG "from temp: %d", REG_READ(sc2->sc_ah, AR_CCCNT));
    }

}


// static long getMicrotime(void){
//     struct timeval currentTime;
//     do_gettimeofday(&currentTime);
//     return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
// }



void get_stats(void)
{
    unsigned int len = 0;
    u32 cycles, busy, rx, tx;
    u8 *buf;
    int i, j = 0;
    unsigned long num_regs, regdump_len, max_reg_offset;
    static const struct reg_hole
    {
        u32 start;
        u32 end;
    } reg_hole_list[] =
    {
        {0x0200, 0x07fc},
        {0x0c00, 0x0ffc},
        {0x2000, 0x3ffc},
        {0x4100, 0x6ffc},
        {0x705c, 0x7ffc},
        {0x0000, 0x0000}
    };

    max_reg_offset = AR_SREV_9300_20_OR_LATER(sc2->sc_ah) ? 0x8800 : 0xb500;
    num_regs = max_reg_offset / 4 + 1;
    regdump_len = num_regs * REGDUMP_LINE_SIZE + 1;
    buf = vmalloc(regdump_len);
    if (!buf)
        return -ENOMEM;

    ath9k_ps_wakeup(sc2);
    if (debug_level2 >= 1)
    {
        printk(KERN_DEBUG "[ath9k][debug.c][open_file_regdump]\n");
    }

    for (i = 0; i < num_regs; i++)
    {
        if (reg_hole_list[j].start == i << 2)
        {
            i = reg_hole_list[j].end >> 2;
            j++;
            continue;
        }

        if (debug_level2 >= 1)
        {
            printk(KERN_DEBUG "from open_file_regdump: 0x%06x\n", i << 2);
            printk(KERN_DEBUG "\t\t: 0x%08x\n", REG_READ(sc2->sc_ah, i << 2));
        }
    }



    cycles = REG_READ(sc2->sc_ah, AR_CCCNT);
    busy = REG_READ(sc2->sc_ah, AR_RCCNT);
    rx = REG_READ(sc2->sc_ah, AR_RFCNT);
    tx = REG_READ(sc2->sc_ah, AR_TFCNT);

    if (debug_level2 >= 1)
    {
        printk(KERN_DEBUG "cycles %d", cycles);
        printk(KERN_DEBUG "busy %d", busy);
        printk(KERN_DEBUG "rx %d", rx);
        printk(KERN_DEBUG "tx %d", tx);
        printk(KERN_DEBUG "NO. of Beacons %d", REG_READ(sc2->sc_ah, 0x8098));
        printk(KERN_DEBUG "NAV value %d", REG_READ(sc2->sc_ah, 0x8084));
    }
    ath9k_ps_restore(sc2);


}



char  *get_stats_ath_update_survey_stats(void)
{
    char *outStr =  kmalloc (sizeof (char) * 500, GFP_KERNEL);

    printk(KERN_DEBUG "[ath9k][link][ath_update_survey_stats]: \n");
    struct ath_hw *ah = sc2->sc_ah;
    struct ath_common *common = ath9k_hw_common(ah);
    int pos = ah->curchan - &ah->channels[0];
    struct survey_info *survey = &sc2->survey[pos];
    struct ath_cycle_counters *cc = &common->cc_survey;
    unsigned int div = common->clockrate * 1000;
    int ret = 0;

    if (!ah->curchan)
        return -1;

    if (ah->power_mode == ATH9K_PM_AWAKE)
        ath_hw_cycle_counters_update(common);

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
        snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx);
        if (debug_level2 >= 1)
        printk(KERN_DEBUG "String to be sent back in json: %s\n", outStr);
    }

    if (cc->cycles < div)
        return -1;

    if (cc->cycles > 0)
        ret = cc->rx_busy * 100 / cc->cycles;

    memset(cc, 0, sizeof(*cc));

    ath_update_survey_nf(sc2, pos);
    return outStr;

}

// uint64_t GetTimeStamp() {
//     struct timeval tv;
//     gettimeofday(&tv,NULL);
//     return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
// }

//EXPORT_SYMBOL(temp);

static void NETLINK_TEST2_recv_msg(struct sk_buff *skb)
{
    int total_kernel_interations = 5;
    int current_kernel_interations = 0;

    int count = 0;

    struct sk_buff *skb_out;
    struct nlmsghdr *nlh;
    int msg_size;
    char *msg;
    int pid;
    int res;

    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid; /* pid of sending process */
    msg = (char *)nlmsg_data(nlh);
    msg_size = strlen(msg);

    long abs_time = getMicrotime();

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
    strcpy(str1, msg);
    if (debug_level2 >= 1)
    {
        printk(KERN_DEBUG "Original string: '%s'\n", str1);
    }
    while( (found = strsep(&str1, " ")) != NULL )
    {
        if (debug_level2 >= 1)
        {
            printk(KERN_DEBUG  "%s\n", found);
        }
        if(count == 0 )
        {
            rett = kstrtoint(found, 10, &sample_duration);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "var: %d\n", sample_duration);
            }
        }

        if(count == 1 )
        {
            rett = kstrtoint(found, 10, &total_kernel_interations);
            {
            	 if (debug_level2 >= 1)
                printk(KERN_DEBUG "var: %d\n", total_kernel_interations);
            }
        }

        if(count == 2 )
        {
            rett = kstrtoint(found, 0, &one);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one: %x\n", one);
            }
            // one_reg_val = REG_READ(sc->sc_ah, one);
            // printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);

        }
        if(count == 3 )
        {
            rett = kstrtoint(found, 0, &two);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "two: %x\n", two);
            }
            // two_reg_val = REG_READ(sc->sc_ah, two);
            // printk(KERN_DEBUG "two_reg_val: %d\n", two_reg_val);
        }
        if(count == 4 )
        {
            rett = kstrtoint(found, 0, &three);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "three: %x\n", three);
            }
            // three_reg_val = REG_READ(sc->sc_ah, three);
            // printk(KERN_DEBUG "three_reg_val: %d\n", three_reg_val);
        }
        if(count == 5 )
        {
            rett = kstrtoint(found, 0, &four);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "four: %d\n", four);
            }
            // four_reg_val = REG_READ(sc->sc_ah, four);
            // printk(KERN_DEBUG "four_reg_val: %d\n", four_reg_val);
        }
        if(count == 6 )
        {
            rett = kstrtoint(found, 0, &five);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "five: %d\n", five);
            }
            // five_reg_val = REG_READ(sc->sc_ah, five);
            // printk(KERN_DEBUG "five_reg_val: %d\n", five_reg_val);
        }
        if(count == 7 )
        {
            rett = kstrtoint(found, 0, &six);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "six: %d\n", six);
            }
            debug_level2 = six;
            // five_reg_val = REG_READ(sc->sc_ah, five);
            // printk(KERN_DEBUG "five_reg_val: %d\n", five_reg_val);
        }




        count = count + 1;
    }
  //   	mutex_lock(&sc->mutex);
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
        if (debug_level2 >= 1)
        {
            printk(KERN_DEBUG "Timestamp [1]: %s\n", temp1);
        }

        if (count == 3)
        {
            one_reg_val = REG_READ(sc2->sc_ah, one);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }

        }
        else if (count == 4)
        {
            one_reg_val = REG_READ(sc2->sc_ah, one);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }
            two_reg_val = REG_READ(sc2->sc_ah, two);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
            }

        }
        else if (count == 5)
        {
            one_reg_val = REG_READ(sc2->sc_ah, one);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }
            two_reg_val = REG_READ(sc2->sc_ah, two);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
            }
            three_reg_val = REG_READ(sc2->sc_ah, three);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", three_reg_val);
            }

        }
        else if (count == 6)
        {
            one_reg_val = REG_READ(sc2->sc_ah, one);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }
            two_reg_val = REG_READ(sc2->sc_ah, two);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
            }
            three_reg_val = REG_READ(sc2->sc_ah, three);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", three_reg_val);
            }

            four_reg_val = REG_READ(sc2->sc_ah, four);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", four_reg_val);
            }

        }


        if (debug_level2 >= 1)
        {
            printk(KERN_DEBUG "the value of count : %d\n", count);
        }

        //[END]parsing the received string for the registers and and fetching the register status.




        // while(k < 1)
        {





            //printk(KERN_DEBUG "from NETLINK_TEST2_recv_msg: %d", REG_READ(sc->sc_ah, AR_CCCNT));
            //get_stats();
            //char buff_to_send[500];
            //char *buff_to_send = get_stats_ath_update_survey_stats();

            char *outStr =  kmalloc (sizeof (char) * 500, GFP_KERNEL);
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "[ath9k][link][ath_update_survey_stats]: \n");
            }
            struct ath_hw *ah = sc2->sc_ah;
            struct ath_common *common = ath9k_hw_common(ah);
            int pos = ah->curchan - &ah->channels[0];
            struct survey_info *survey = &sc2->survey[pos];
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
            if (debug_level2 >= 1)
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
            if (debug_level2 >= 1)
            {
                printk(KERN_INFO "[1]NETLINK_TEST2: Received from pid %d: %s\n", pid, msg);
            }
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
            if (debug_level2 >= 1)
            {
                printk(KERN_DEBUG "String to be sent back in json: %s\n", outStr);
            }
            //if (cc->cycles < div)
            //	return -1;

            if (cc->cycles > 0)
                ret = cc->rx_busy * 100 / cc->cycles;

            memset(cc, 0, sizeof(*cc));

            ath_update_survey_nf(sc2, pos);// move this before preparing the string
            //return outStr;



            // create reply
            skb_out = nlmsg_new(strlen(outStr), 0);
            if (!skb_out)
            {
                if (debug_level2 >= 1)
                {
                    printk(KERN_ERR "NETLINK_TEST2: Failed to allocate new skb\n");
                }
                return;
            }




            // put received message into reply
            nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, strlen(outStr), 0);
            NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
            strncpy(nlmsg_data(nlh), outStr, strlen(outStr));

            if (debug_level2 >= 1)
            {
                printk(KERN_INFO "NETLINK_TEST2: Send %s\n", outStr);
            }
            res = nlmsg_unicast(nl_sock, skb_out, pid);
            if (res < 0)
                printk(KERN_INFO "NETLINK_TEST2: Error while sending skb to user, err code %d\n", res);

            kfree(outStr);
            k = k + 1;
            // if (debug_level2 >= 1)
            // {
            //     printk(KERN_INFO "Sleeping %s\n");
            // }
            //             if (sample_duration<(1000000))
            //     usleep_range(sample_duration,sample_duration+1);
            // else if (sample_duration>=(1000000))
            //     msleep(sample_duration/1000);
            // else
            // {
            //     if (debug_level >= 1)
            //         {
            //             printk(KERN_INFO "Unable to Sleep %s\n");
            //         }
            // }
            // 
            abs_time = abs_time + sample_duration;
            int sleep_duration;
            int jj = 9;
            if (jj == 9)
            {
                
                long now_time = getMicrotime() ;
                if ((abs_time - now_time)>0)
                    sleep_duration = abs_time - now_time;
                else
                    sleep_duration = 0;
                if (debug_level >= 1)
                printk(KERN_INFO "abstime:%ld, now time:%ld, delta:%d, Sleeping for:%d\n",abs_time, now_time, abs_time - now_time, sleep_duration);
            }
            // prev_sample_duration = sample_duration;

            if(sleep_duration >0){
            if (sleep_duration<(1000000))
                usleep_range(sleep_duration,sleep_duration+1);
            else if (sleep_duration>=(1000000))
                msleep(sleep_duration/1000);
            else
            {
                if (debug_level >= 1)
                    {
                        printk(KERN_INFO "Unable to Sleep %s\n");
                    }
            }
        }
            // 
        }
    }
}
static int NETLINK_TEST2_init(void)
{
    printk(KERN_INFO "NETLINK_TEST2: Init module\n");

    struct netlink_kernel_cfg cfg =
    {
        .input = NETLINK_TEST2_recv_msg,
    };

    nl_sock = netlink_kernel_create(&init_net, NETLINK_TEST2, &cfg);
    if (!nl_sock)
    {
        printk(KERN_ALERT "NETLINK_TEST2: Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void NETLINK_TEST2_exit(void)
{
    printk(KERN_INFO "NETLINK_TEST2: Exit module\n");

    netlink_kernel_release(nl_sock);
}


