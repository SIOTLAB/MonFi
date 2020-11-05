#!/bin/bash
while :
do
	sudo echo > /dev/null | sudo tee /var/log/syslog.1
	sudo echo > /dev/null | sudo tee /var/log/syslog
	sudo echo > /dev/null | sudo tee /var/log/kern.log
	sudo echo > /dev/null | sudo tee /var/log/kern.log.1
	echo "flushed"
	sleep 20m
done
