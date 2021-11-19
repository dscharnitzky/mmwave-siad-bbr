import numpy as np
import os


tp_bbr = []
rtt_bbr = []
tp_siad = []
rtt_siad = []

for i in range(1, 31, 2):
	#read data for BBR throughput
	f = open('../traces/7_20_40/%s/processedwindow.txt' % i, 'r')
	lines = f.readlines()
	for line in lines:
		time, tp = line.split('; ')
		tp_bbr.append(8 * float(tp))

	#read data for BBR RTT
	f = open('../traces/7_20_40/%s/mmWave-tcp-rtt0.txt' % i, 'r')
	lines = f.readlines()
	for line in lines:
		time, rtt0, rtt1 = line.split('\t')
		rtt_bbr.append(1000 * float(rtt1))

	#read data for SIAD throughput
	j = i + 1 
	f = open('../traces/7_20_40/%s/processedwindow.txt' % j, 'r')
	lines = f.readlines()
	for line in lines:
		time, tp = line.split('; ')
		tp_siad.append(8 * float(tp))

	#read data for SIAD RTT
	f = open('../traces/7_20_40/%s/mmWave-tcp-rtt0.txt' % j, 'r')
	lines = f.readlines()
	for line in lines:
		time, rtt0, rtt1 = line.split('\t')
		rtt_siad.append(1000 * float(rtt1))



#Calculate average throughput and standard deviation
avg_tp_bbr = np.mean(tp_bbr)
avg_tp_siad = np.mean(tp_siad)
#std_tp_bbr = np.std(tp_bbr)
#std_tp_siad = np.std(tp_siad)

#print("BBR throughput: ", avg_tp_bbr, "+- ", std_tp_bbr)
#print("SIAD throughput: ", avg_tp_bbr, "+- ", std_tp_siad)
print("Average BBR throughput: ", avg_tp_bbr, "Mbps")
print("Average SIAD throughput: ", avg_tp_siad, "Mbps")

#Calculate average RTT
avg_rtt_bbr = np.mean(rtt_bbr)
avg_rtt_siad = np.mean(rtt_siad)

print("Average BBR RTT: ", avg_rtt_bbr, "ms")
print("Average SIAD RTT: ", avg_rtt_siad, "ms")
