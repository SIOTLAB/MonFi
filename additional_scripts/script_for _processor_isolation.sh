#!/bin/bash 

###################################################################
#Name		:script_for_processor_isolation.sh	                                                                                              
#Description	:sets the SMP affinity of all kernel processes that interupt the CPU                                                                                    
#Args		:ath9k OR ath10k                                                                            
#Author       	:Jay Sheth                                                
#Email         	:jsheth2090@gmail.com                                           
###################################################################


# cat /proc/interrupts ... mainatains the stats about the number of interuppt from each device and the CPU that serverd the interrupt
# /proc/irq/* contains the attributes of all the devices 
# cat /proc/softirqs
# cat /proc/stat

temp=`cat /proc/cpuinfo | grep "core id" | cut -d ":" -f 2`
echo $temp 
cpu_info1=`echo $temp | cut -d " " -f 1`
cpu_info2=`echo $temp | cut -d " " -f 2`
cpu_info3=`echo $temp | cut -d " " -f 3`
cpu_info4=`echo $temp | cut -d " " -f 4`
echo $cpu_info1 
echo $cpu_info2 
echo $cpu_info3 
echo $cpu_info4 

if [[ "$cpu_info1" == 0 ]] && [[ "$cpu_info2" == 1 ]] && [[ "$cpu_info3" == 0 ]] && [[ "$cpu_info4" == 1 ]]
then
	    echo "CPU0 and CPU2 are on the same physical core and CPU1 and CPU3 are on the same physical core"
else
		echo "FALSE"
fi


#we have isolated core 1 and 3 from all user space processes, but its still prone to kernel processes
FILES=/proc/irq/*

k_id=`cat /proc/interrupts |grep $1 | grep -o -P '.{1,2}:' | cut -d ":" -f 1`
echo $k_id

echo "We will pin the $1 module to CPU 3, all the other kernel modules to CPU 0 and 2. We will then pin the WatchAP userspace measurement module to CPU1"
for f in $FILES
do
	echo "Processing $f file..."
	foo="${f}/smp_affinity"
	file1=`cat $foo`

	file_k_id=`echo ${f} | cut -d "/" -f 4`

	# echo $file_k_id
	if [[ "$k_id" == "$file_k_id" ]]
	then
	    echo "*******MATCHED*******Current CPU affinity of: $foo is: $file1"
	    echo 2 > $foo
	    echo "*******MATCHED MODIFIED*******Current CPU affinity of: $foo is: $file1"
	else
	   echo "...................Current CPU affinity of: $foo is: $file1"
	   echo 5 > $foo
	   echo "...................Modified CPU affinity of: $foo is: $file1"
	fi
	# echo "...................Current CPU affinity of: $foo is: $file1"
	# take action on each file. $f store current file name
	# cat $f
done

#             Binary       Hex 
#     CPU 0    0001         1 
#     CPU 1    0010         2
#     CPU 2    0100         4
#     CPU 3    1000         8

# By combining these bit patterns (basically, just adding the Hex values), we
# can address more than one processor at a time.   For example, if I wanted
# to talk to both CPU0 and CPU2 at the same time, the result is:

#             Binary       Hex 
#     CPU 0    0001         1 
#   + CPU 2    0100         4
#     -----------------------
#     both     0101         5

# If I want to address all four of the processors at once, then the result is:

#             Binary       Hex 
#     CPU 0    0001         1 
#     CPU 1    0010         2
#     CPU 2    0100         4
#   + CPU 3    1000         8
#     -----------------------
#     both     1111         f
