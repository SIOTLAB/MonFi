#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/netlink.h>


// struct sock *nl_sk = NULL;
// struct ath_softc *sc;
int debug_level = 9;
// char *temp_message = kmalloc (sizeof (char) * 500, GFP_KERNEL);
char temp_message[1024];
u32 rx_clear_count_collected;
u32 cycle_count_collected;
u32 reg_val;
struct ath10k *ar; 
struct ath10k *ar1;
static void ath10k_debug_fw_stats_pdevs_free(struct list_head *head)
{
    struct ath10k_fw_stats_pdev *i, *tmp;

    list_for_each_entry_safe(i, tmp, head, list) {
        list_del(&i->list);
        kfree(i);
    }
}

static void ath10k_debug_fw_stats_vdevs_free(struct list_head *head)
{
    struct ath10k_fw_stats_vdev *i, *tmp;

    list_for_each_entry_safe(i, tmp, head, list) {
        list_del(&i->list);
        kfree(i);
    }
}

static void ath10k_debug_fw_stats_peers_free(struct list_head *head)
{
    struct ath10k_fw_stats_peer *i, *tmp;

    list_for_each_entry_safe(i, tmp, head, list) {
        list_del(&i->list);
        kfree(i);
    }
}

static void ath10k_debug_fw_stats_reset(struct ath10k *ar)
{
    spin_lock_bh(&ar->data_lock);
    ar->debug.fw_stats_done = false;
    ath10k_debug_fw_stats_pdevs_free(&ar->debug.fw_stats.pdevs);
    ath10k_debug_fw_stats_vdevs_free(&ar->debug.fw_stats.vdevs);
    ath10k_debug_fw_stats_peers_free(&ar->debug.fw_stats.peers);
    spin_unlock_bh(&ar->data_lock);
}

static int ath10k_debug_fw_stats_request(struct ath10k *ar)
{
    unsigned long timeout, time_left;
    int ret;

    lockdep_assert_held(&ar->conf_mutex);

    timeout = jiffies + msecs_to_jiffies(1 * HZ);

    ath10k_debug_fw_stats_reset(ar);

    for (;;) {
        if (time_after(jiffies, timeout))
            return -ETIMEDOUT;

        reinit_completion(&ar->debug.fw_stats_complete);

        ret = ath10k_wmi_request_stats(ar, ar->fw_stats_req_mask);
        if (ret) {
            ath10k_warn(ar, "could not request stats (%d)\n", ret);
            return ret;
        }

        time_left =
        wait_for_completion_timeout(&ar->debug.fw_stats_complete,
                        1 * HZ);
        if (!time_left)
            return -ETIMEDOUT;

        spin_lock_bh(&ar->data_lock);
        if (ar->debug.fw_stats_done) {
            spin_unlock_bh(&ar->data_lock);
            break;
        }
        spin_unlock_bh(&ar->data_lock);
    }

    return 0;
}


void collect_chanU1(void)
{
    if (debug_level >= 1)
    {
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][ax]%x\n", ath10k_hif_read32(ar1, ar1->debug.reg_addr));
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][ad]%d\n", ath10k_hif_read32(ar1, ar1->debug.reg_addr));
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][bx]%08x\n", ar1->debug.reg_addr);
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][bd]%d\n", ar1->debug.reg_addr);
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1]%08x\n", 0x000280f8);
    }
    cycle_count_collected = ath10k_hif_read32(ar1, 164088);
    if (debug_level >= 1)
    {
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][cx]%08x\n", ath10k_hif_read32(ar1, 164088));
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][cd]%d\n", ath10k_hif_read32(ar1, 164088));
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][d]%d\n", cycle_count_collected);
    printk(KERN_DEBUG "[ath10k][kernel_netlink_hunter_ath10k.c][collect_chanU1][x]%x\n", cycle_count_collected);
    }
    rx_clear_count_collected = ath10k_hif_read32(ar1, 164084);
    udelay(10);
}

void collect_chanU(void)
{
        void *buf = NULL;
    int ret;
u32 len = 0;
    u32 buf_len = ATH10K_FW_STATS_BUF_SIZE;
    const struct ath10k_fw_stats_pdev *pdev;
    const struct ath10k_fw_stats_vdev *vdev;
    const struct ath10k_fw_stats_peer *peer;
    size_t num_peers;
    size_t num_vdevs;
    // mutex_lock(&ar->conf_mutex);

    if (ar->state != ATH10K_STATE_ON) {
        ret = -ENETDOWN;
        printk("[1]err_unlock\n");
        // goto err_unlock;
    }

    buf = vmalloc(ATH10K_FW_STATS_BUF_SIZE);
    if (!buf) {
        ret = -ENOMEM;
        printk("[1]err_unlock\n");
        // goto err_unlock;
    }

    ret = ath10k_debug_fw_stats_request(ar);
    if (ret) {
        ath10k_warn(ar, "failed to request fw stats: %d\n", ret);
        printk("[1]err_free\n");
        // goto err_free;
    }

    ret = ath10k_wmi_fw_stats_fill(ar, &ar->debug.fw_stats, buf);
    if (ret) {
        ath10k_warn(ar, "failed to fill fw stats: %d\n", ret);
        printk("[1]err_free\n");
    }

    // file->private_data = buf;
    pdev = list_first_entry_or_null(&ar->debug.fw_stats.pdevs,
                    struct ath10k_fw_stats_pdev, list);
    if (debug_level >= 1)
    {
       printk("[1]kernel start recv from user-ath10k:");
       printk("[1]kernel start recv from user-ath10k value: %d\n",pdev->rx_clear_count);
       printk("[1]kernel start recv from user-ath10k:1");
       printk("[2]kernel start recv from user-ath10k:");
       printk("[2]kernel start recv from user-ath10k value: %u\n",pdev->cycle_count);
       printk("[2]kernel start recv from user-ath10k:1");
   }
       rx_clear_count_collected = pdev->rx_clear_count;
       cycle_count_collected = pdev->cycle_count;

}

void temp(struct inode *inode)
{
    // sc = inode->i_private;
    //if (debug_level >= 1)
    ar1 = inode->i_private;
    // ar1 = file->private_data;
    {
        printk(KERN_INFO "this is the temp function-ath10k\n");
        // printk(KERN_DEBUG "from temp-ath10k: %d", REG_READ(sc->sc_ah, AR_CCCNT));

        // const struct ath10k_fw_stats_pdev *pdev;
        // pdev = list_first_entry_or_null(&ar->debug.fw_stats.pdevs,
        //             struct ath10k_fw_stats_pdev, list);
        // ath10k_wmi_fw_pdev_base_stats_fill(pdev, buf, &len);

//         u32 len = 0;
//     u32 buf_len = ATH10K_FW_STATS_BUF_SIZE;
//     const struct ath10k_fw_stats_pdev *pdev;
//     const struct ath10k_fw_stats_vdev *vdev;
//     const struct ath10k_fw_stats_peer *peer;
//     size_t num_peers;
//     size_t num_vdevs;


//     pdev = list_first_entry_or_null(&ar->debug.fw_stats.pdevs,
//                     struct ath10k_fw_stats_pdev, list);
//     if (debug_level >= 1){
// 	   printk("kernel start recv from user-ath10k:");
//        printk("kernel start recv from user-ath10k value: %d\n",pdev->rx_clear_count);
//        printk("kernel start recv from user-ath10k:1");
// }
//        collect_chanU();
//        usleep_range(1000,1050);
//        collect_chanU();
//        usleep_range(1000000,1000100);
//        collect_chanU();

    // void *buf = NULL;
    // int ret;

    // // mutex_lock(&ar->conf_mutex);

    // if (ar->state != ATH10K_STATE_ON) {
    //     ret = -ENETDOWN;
    //     printk("[1]err_unlock\n");
    //     // goto err_unlock;
    // }

    // buf = vmalloc(ATH10K_FW_STATS_BUF_SIZE);
    // if (!buf) {
    //     ret = -ENOMEM;
    //     printk("[1]err_unlock\n");
    //     // goto err_unlock;
    // }

    // ret = ath10k_debug_fw_stats_request(ar);
    // if (ret) {
    //     ath10k_warn(ar, "failed to request fw stats: %d\n", ret);
    //     printk("[1]err_free\n");
    //     // goto err_free;
    // }

    // ret = ath10k_wmi_fw_stats_fill(ar, &ar->debug.fw_stats, buf);
    // if (ret) {
    //     ath10k_warn(ar, "failed to fill fw stats: %d\n", ret);
    //     printk("[1]err_free\n");
    // }

    // // file->private_data = buf;
    // pdev = list_first_entry_or_null(&ar->debug.fw_stats.pdevs,
    //                 struct ath10k_fw_stats_pdev, list);
    //    printk("[1]kernel start recv from user-ath10k:");
    //    printk("[1]kernel start recv from user-ath10k value: %d\n",pdev->rx_clear_count);
    //    printk("[1]kernel start recv from user-ath10k:1");

    }

}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("hunter");

#define MAX_MSGSIZE 125
#define NL_FR_SZ    16384
#define TDB_NLMSG_MAXSZ		(NL_FR_SZ / 2 - NLMSG_HDRLEN - MAX_MSGSIZE \
				 - MAX_MSGSIZE)

#define NETLINK_TEST 30

struct sock *nl_sk = NULL;


static long getMicrotime(void){
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

static int tdb_if_proc_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    char kmsg[] = "hello users!!!";
    char *m = NULL;
    m = nlmsg_data(nlh);
    if (debug_level >= 1)
    {
    if(m)
    {
        printk("kernel recv from user: %s\n", m);
    }
    }
    long abs_time = getMicrotime();
    
    // usleep_range(100,102);
    // printk("Time 1 : %ld\n", getMicrotime()- abs_time);

    // abs_time = getMicrotime();
    // usleep_range(1000,1002);
    // printk("Time 2 : %ld\n", getMicrotime()- abs_time);

    // abs_time = getMicrotime();
    // usleep_range(500,502);
    // printk("Time 3 : %ld\n", getMicrotime()- abs_time);
    
// ----------------------------------------
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
    int prev_sample_duration = 0;
    int one, two, three, four, five,six;
    int one_reg_val, two_reg_val, three_reg_val, four_reg_val, five_reg_val;

    char *string, *found;

    //string = strdup("Hello there, peasants!");
    char *str1 = kmalloc(sizeof(char) * 254, GFP_KERNEL);
    strcpy(str1, m);
    if (debug_level >= 1)
    {
        printk(KERN_DEBUG "Original string: '%s'\n", str1);
    }
    // ((((((((((((((((((((((((((((((((((((((()))))))))))))))))))))))))))))))))))))))
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
                printk(KERN_DEBUG "sample_duration: %d\n", sample_duration);
                if (sample_duration == 0)
                    sample_duration = prev_sample_duration;
                printk(KERN_DEBUG "final sample_duration: %d\n", sample_duration);
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
    if (debug_level >= 1)
        {
            printk(KERN_DEBUG "total_kernel_interations : %d\n", total_kernel_interations);
        }

    while (current_kernel_interations < total_kernel_interations)
    {
        if (debug_level >= 1)
        {
            printk(KERN_DEBUG "current_kernel_interations : %d\n", current_kernel_interations);
        }
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
            one_reg_val = 99;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }

        }
        else if (count == 4)
        {
            one_reg_val = 990;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }
            two_reg_val = 991;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
            }

        }
        else if (count == 5)
        {
            one_reg_val = 990;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }
            two_reg_val = 991;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
            }
            three_reg_val = 992;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", three_reg_val);
            }

        }
        else if (count == 6)
        {
            one_reg_val = 990;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", one_reg_val);
            }
            two_reg_val = 991;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", two_reg_val);
            }
            three_reg_val = 992;
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "one_reg_val: %d\n", three_reg_val);
            }

            four_reg_val = 993;
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
            // struct ath_hw *ah = sc->sc_ah;
            // struct ath_common *common = ath9k_hw_common(ah);
            // int pos = ah->curchan - &ah->channels[0];
            // struct survey_info *survey = &sc->survey[pos];
            // struct ath_cycle_counters *cc = &common->cc_survey;
            // unsigned int div = common->clockrate;
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


            // if (!ah->curchan)
            //     return -1;

            if(count != 2)
            {
                // if (ah->power_mode == ATH9K_PM_AWAKE)
                //     ath_hw_cycle_counters_update(common);
                // raw_local_irq_restore(flags); /*we enable hard interrupts on our CPU*/
                // preempt_enable();/*we enable preemption*/

                // if (cc->cycles > 0)
                // {

                //     survey->filled |= SURVEY_INFO_TIME |
                //                       SURVEY_INFO_TIME_BUSY |
                //                       SURVEY_INFO_TIME_RX |
                //                       SURVEY_INFO_TIME_TX;
                //     survey->time += cc->cycles / div;
                //     survey->time_busy += cc->rx_busy / div;
                //     survey->time_rx += cc->rx_frame / div;
                //     survey->time_tx += cc->tx_frame / div;
                    
                //     printk(KERN_DEBUG "cc->cycles: %lu\n", cc->cycles);
                //     printk(KERN_DEBUG "cc->rx_busy: %lu\n", cc->rx_busy);
                //     printk(KERN_DEBUG "cc->rx_frame: %lu\n", cc->rx_frame);
                //     printk(KERN_DEBUG "cc->tx_frame: %lu\n", cc->tx_frame);
                //     printk(KERN_DEBUG "survey->time: %lu\n", survey->time);
                //     printk(KERN_DEBUG "survey->time_busy: %lu\n", survey->time_busy);
                //     printk(KERN_DEBUG "survey->time_rx: %lu\n", survey->time_rx);
                //     printk(KERN_DEBUG "survey->time_tx: %lu\n", survey->time_tx);
                //     printk(KERN_DEBUG "common->clockrate: %lu\n", common->clockrate);
                    

                //     int intvar = 100;
                //     //char str[16];
                //     // '{"name":"John", "age":31, "city":"New York"}'
                //     //printk(KERN_DEBUG "sizeof(outStr): %d\n", sizeof(outStr));

                // }
            }
            // if (debug_level >= 1)
            // {
            //     printk(KERN_INFO "[1]netlink_test: Received from pid %d: %s\n", pid, msg);
            // }
            if (debug_level == 8){
            struct timespec ts21551;
            getnstimeofday(&ts21551);
            printk("CHANU time %ld.%ld seconds\n", ts21551.tv_sec, ts21551.tv_nsec);
            }
            collect_chanU1();
            if (debug_level == 8)
            {
            struct timespec ts21552;
            getnstimeofday(&ts21552);
            
            printk("CHANU time %ld.%ld seconds\n", ts21552.tv_sec, ts21552.tv_nsec);
            }
            rx_clear_count_collected=10;
            cycle_count_collected=11;
            if(count == 2)
            {
                snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 100, 100, 100, 100, 100, 100, 100, 100, one, ts2.tv_sec, ts2.tv_nsec);

            }

            if(count == 3)
            {
                snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 101, 102, 103, 104, cycle_count_collected, rx_clear_count_collected, 201, 202, one, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
            }
            else if(count == 4)
            {
                snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 101, 102, 103, 104, cycle_count_collected, rx_clear_count_collected, 201, 202, one, two, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
            }
            else if(count == 5)
            {
                snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 101, 102, 103, 104, cycle_count_collected, rx_clear_count_collected, 201, 202, one, two, three, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
            }
            else if(count == 6)
            {
                snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"four\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 101, 102, 103, 104, cycle_count_collected, rx_clear_count_collected, 201, 202, one, two, three, four, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
            }
            else if(count == 7)
            {
                snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"four\":%d, \"five\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 101, 102, 103, 104, cycle_count_collected, rx_clear_count_collected, 201, 202, one, two, three, four, five, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
            }
            else
            {
                snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d, \"one\":%d, \"two\":%d, \"three\":%d, \"four\":%d, \"five\":%d, \"Timestamp1\":%ld.%ld, \"Timestamp2\":%ld.%ld}", 101, 102, 103, 104, cycle_count_collected, rx_clear_count_collected, 201, 202, one, two, three, four, five, ts1.tv_sec, ts1.tv_nsec, ts2.tv_sec, ts2.tv_nsec);
            }




            //snprintf(outStr, 499, "{\"cc->cycles\":%d, \"cc->rx_busy\":%d, \"cc->rx_frame\":%d, \"cc->tx_frame\":%d, \"survey->time\":%d, \"survey->time_busy:\":%d, \"survey->time_rx\":%d, \"survey->time_tx\":%d}", cc->cycles, cc->rx_busy, cc->rx_frame, cc->tx_frame, survey->time, survey->time_busy, survey->time_rx, survey->time_tx);
            if (debug_level >= 1)
            {
                printk(KERN_DEBUG "String to be sent back in json: %s\n", outStr);
            }
            //if (cc->cycles < div)
            //  return -1;

            // if (cc->cycles > 0)
            //     ret = cc->rx_busy * 100 / cc->cycles;

            // memset(cc, 0, sizeof(*cc));

            // ath_update_survey_nf(sc, pos);// move this before preparing the string
            // //return outStr;



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



            if (debug_level >= 1)
            printk(KERN_INFO "temp_message: Send %s\n", temp_message);

            struct netlink_dump_control c = {
                .dump = tdb_if_info,
                //.data = m,
                .min_dump_alloc = NL_FR_SZ / 2,
            };
            netlink_dump_start(nl_sk, skb, nlh, &c);






            kfree(outStr);
            k = k + 1;

            

            char *temp21;
            temp21 = kmalloc(50 * sizeof(char), GFP_KERNEL);
            struct timespec ts21;
            getnstimeofday(&ts21);
            sprintf(temp21, "%ld.%ld seconds\n", ts21.tv_sec, ts21.tv_nsec);
            // sprintf(temp2, "%ld.%ld seconds\n", ts2.tv_sec, ts2.tv_nsec);
            // if (debug_level >= 1)
            // {
            //     printk(KERN_DEBUG "Timestamp [211]: %s\n", temp21);
            // }
            // if (debug_level >= 1)
            // {
            //     printk(KERN_DEBUG "Timestamp [21]: %lu\n", (ts21.tv_nsec)/1000);
            // }
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
                if (debug_level == 7)
                printk(KERN_INFO "abstime:%ld, now time:%ld, delta:%d, Sleeping for:%d\n",abs_time, now_time, abs_time - now_time, sleep_duration);
            }
            prev_sample_duration = sample_duration;

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
            
            // struct timespec ts31;
            // char *temp31;
            // temp31 = kmalloc(50 * sizeof(char), GFP_KERNEL);
            // getnstimeofday(&ts31);
            // sprintf(temp31, "%ld.%ld seconds\n", ts31.tv_sec, ts31.tv_nsec);
            // if (debug_level >= 1)
            // {
            //     printk(KERN_DEBUG "Timestamp [31]: %lu\n", (ts31.tv_nsec)/1000);
            // }
            // if (debug_level >= 1)
            // {
            //     printk(KERN_DEBUG "Timestamp [311]: %ld\n", ts31.tv_nsec- ts21.tv_nsec);
            // }
            // if (debug_level >= 1)
            // {
            //     printk(KERN_DEBUG "Timestamp [3111]: %s\n", temp31);
            // }

        }
    }
}
//----------------------------------------



    return 1 ;
}

static void tdb_if_rcv(struct sk_buff *skb)
{
	/* TODO remove the mutex for concurrent user-space updates. */
        struct nlmsghdr *nlh = NULL;
	nlh = nlmsg_hdr(skb);
        tdb_if_proc_msg(skb, nlh);
}

struct netlink_kernel_cfg cfg = {
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

// module_init(myinit_module);
// module_exit(mycleanup_module);
