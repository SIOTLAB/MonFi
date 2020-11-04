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
- We recommend using the complete kernel image as memory-mapped netlink is not supported in some of the newer kernel versions by default [[1]](http://natsys-lab.blogspot.com/2015/03/linux-netlink-mmap-bulk-data-transfer.html). 
- However, most of the implementation is inside the Atheros driver [[Link]](https://github.com/SIOTLAB/MonFi/tree/main/linux-4.5.1/drivers/net/wireless/ath)
  - Specifically, in [ath9k](https://github.com/SIOTLAB/MonFi/tree/main/linux-4.5.1/drivers/net/wireless/ath/ath9k) and [ath10k](https://github.com/SIOTLAB/MonFi/tree/main/linux-4.5.1/drivers/net/wireless/ath/ath10k) folders
  - Initally, we use the Atheros-debugfs to invoke the Collector and sharing of Atheros data structure which can then be utilized for collecting measurements on receiving instructions from the Controller.




