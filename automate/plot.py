import numpy as np
import matplotlib.pyplot as plt
import os

for i in range(1, 30, 2):
	#read data for BBR throughput
	tp_x_bbr = []
	tp_y_bbr = []
	f = open('../traces/%s/processedwindow.txt' % i, 'r')
	lines = f.readlines()
	for line in lines:
		time, tp = line.split('; ')
		tp_x_bbr.append(float(time))
		tp_y_bbr.append(float(tp) * 8.0)

	#read data for BBR RTT
	rtt_x_bbr = []
	rtt_y_bbr = []
	f = open('../traces/%s/mmWave-tcp-rtt0.txt' % i, 'r')
	lines = f.readlines()
	for line in lines:
		time, rtt0, rtt1 = line.split('\t')
		rtt_x_bbr.append(float(time))
		rtt_y_bbr.append(1000 * float(rtt1))

	#read data for SIAD throughput
	tp_x_siad = []
	tp_y_siad = []
	j = i + 1 
	f = open('../traces/%s/processedwindow.txt' % j, 'r')
	lines = f.readlines()
	for line in lines:
		time, tp = line.split('; ')
		tp_x_siad.append(float(time))
		tp_y_siad.append(float(tp) * 8.0)

	#read data for SIAD RTT
	rtt_x_siad = []
	rtt_y_siad = []
	f = open('../traces/%s/mmWave-tcp-rtt0.txt' % j, 'r')
	lines = f.readlines()
	for line in lines:
		time, rtt0, rtt1 = line.split('\t')
		rtt_x_siad.append(float(time))
		rtt_y_siad.append(1000 * float(rtt1))

	# Create two subplots sharing y axis
	fig, ax = plt.subplots(2)

	fig.suptitle('Throughput and RTT over time', fontsize = 20)

	ax[0].plot(tp_x_bbr, tp_y_bbr, 'b-', label='BBR')
	ax[0].plot(tp_x_siad, tp_y_siad, 'g-', label='SIAD')
	ax[0].set(ylabel='Throughput (Mbps)')
	ax[0].grid(True)

	ax[1].plot(rtt_x_bbr, rtt_y_bbr, 'b-')
	ax[1].plot(rtt_x_siad, rtt_y_siad, 'g-')
	ax[1].set(xlabel='Time (s)', ylabel='RTT (ms)')
	ax[1].grid(True)

	#ax2.legend(loc = 'upper right')
	fig.legend(loc="upper right")
	plt.subplots_adjust(top = 0.88)

	plt.savefig('../traces/%s.png' % i, bbox_inches='tight', dpi=300)

