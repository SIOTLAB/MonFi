# MonFi:  Tool for High-Rate, Efficient, and Programmable Monitoring of WiFi Devices

MonFi is a Linux-based, open-source tool for efficient, high-rate, and programmable monitoring of WiFi devices.
This tool allows for monitoring the complete WiFi stack---NIC, driver, mac80211, cfg80211, hostapd, and qdisc.
The monitoring frequency and the type of measurements collected can be programmatically specified using the user-space component of MonFi (a.k.a, Controller).
We present methods to monitor the WiFi stack, and also implement and study methods for reducing the overhead of kernel to user-space communication and stabilizing monitoring rate in the presence of interfering loads on the processor. 


<img src="https://raw.githubusercontent.com/jshethSCU/temp/master/ap_architecture-1.png?token=ANVXY37NZU3M7BSWAFSPS4C7VNKSM" width="700" height="500" align="center">


The **Controller** is a user-space module that configures and receives measurements from the **Collector**.
The Collector is a kernel-space module that collects data across the network stack.
The Collector is implemented as a part of the driver to share a set of functions and data structures.
The Collector also interacts with other modules of the communication stack.


## MonFi Software Components
- The user-space component of MonFi is located inside User_Space_Controller folder as [mmaped_netlink_Controller.c](https://github.com/SIOTLAB/MonFi/blob/main/User_Space_Controller/mmaped_netlink_Controller.c)
- We recommend using the complete kernel image as memory-mapped netlink is not supported in some of the newer kernel versions by default [[1]](http://natsys-lab.blogspot.com/2015/03/linux-netlink-mmap-bulk-data-transfer.html). Furthermore, we have also applied the patch which enables CRDA country code to be switched, and thus allows us to use 5Ghz channels via hostapd in AP mode.  
  - Patch coming soon!
- However, most of the implementation (for the collector) is inside the Atheros driver [[Link]](https://github.com/SIOTLAB/MonFi/tree/main/linux-4.5.1/drivers/net/wireless/ath)
  - Specifically, in [ath9k](https://github.com/SIOTLAB/MonFi/tree/main/linux-4.5.1/drivers/net/wireless/ath/ath9k) and [ath10k](https://github.com/SIOTLAB/MonFi/tree/main/linux-4.5.1/drivers/net/wireless/ath/ath10k) folders
  -  [ath9k_Collector_MNL.c](https://github.com/SIOTLAB/MonFi/blob/main/linux-4.5.1/drivers/net/wireless/ath/ath9k/ath9k_Collector_MNL.c) implements most of the functionalities of Collector kernel-space module.
  - Initally, we use the Atheros-debugfs to invoke the Collector and sharing of Atheros data structure which can then be utilized for collecting measurements on receiving instructions from the Controller.
  - Hence, we need to run "cat /sys/kernel/debug/ieee80211/phy0/ath9k/queues" while initializing MonFi. This will invoke the temp2() function in the ath9k_Collector_MNL.c and pass the pointer to the ath9k data structure to ath9k_Collector_MNL.c.
- [additional_scripts](https://github.com/SIOTLAB/MonFi/tree/main/additional_scripts) folder contains bash scripts, such as:
  - [housekeeping.sh](https://github.com/SIOTLAB/MonFi/blob/main/additional_scripts/housekeeping.sh): If we enable the debug mode for MonFi, it would result in a lot of printk statements being directed to the kernel log. Hence, over time, kernel logs might consume a lot of strorage space. To this end, this script wipes out kernel logs every 20 minutes to avoid disks running out of space.
  - CPU Isolation: 
    * Handling hardware interrupts: [script_for_processor_isolation.sh](https://github.com/SIOTLAB/MonFi/blob/main/additional_scripts/script_for%20_processor_isolation.sh) script helps us in isolating CPU for the execution of Atheros driver and MonFi. First we find out all the components which can generate a hardware intrrupt (via "cat /proc/interrupts") and then set the SMP affinity of all the components to 0 and 2, pin ath9k/ath10k to CPU 1 or 3.     
    * Handling software interrupts and scheduler
      * The grub config file can found via "sudo nano /etc/default". We have included a sample grub config file at [sample_grub_config](https://github.com/SIOTLAB/MonFi/blob/main/additional_scripts/sample_grub_config): 
      * The line (GRUB_CMDLINE_LINUX_DEFAULT="quiet splash isolcpus=1,3") is reposible for isolating core 1 and 3 (note that numbering starts from 0) for MonFi and Atheros driver, i.e., the processor will not schedule any processes on core 2 and 4. CPU isolation turned off --- if line (GRUB_CMDLINE_LINUX_DEFAULT="quiet splash") is uncommented and line (GRUB_CMDLINE_LINUX_DEFAULT="quiet splash isolcpus=1,3") is commented. CPU isolation turned on --- if line (GRUB_CMDLINE_LINUX_DEFAULT="quiet splash") is commented and line (GRUB_CMDLINE_LINUX_DEFAULT="quiet splash isolcpus=1,3") is uncommented.
      * After modifying the file. Run "sudo update-grub", or "sudo /usr/sbin/update-grub" and reboot the machine.
  - hostapd config scripts: [80211_ath9k.conf](https://github.com/SIOTLAB/MonFi/blob/main/additional_scripts/hostapd_80211_ath9k.conf) and  [80211_ath10k.conf](https://github.com/SIOTLAB/MonFi/blob/main/additional_scripts/hostapd_80211_ath10k.conf) 



