import os
import time

mht_file_array = ['OUTPUT_MHT_100.mht', 'OUTPUT_MHT_200.mht', 'OUTPUT_MHT_500.mht', \
				  'OUTPUT_MHT_1000.mht', 'OUTPUT_MHT_2000.mht', 'OUTPUT_MHT_5000.mht', \
				  'OUTPUT_MHT_10000.mht', 'OUTPUT_MHT_20000.mht', 'OUTPUT_MHT_50000.mht', \
				  'OUTPUT_MHT_100000.mht', 'OUTPUT_MHT_200000.mht', 'OUTPUT_MHT_500000.mht']

os.system("make clean-mhtfile")
print('Building MHT files...')
os.system("python runbench_build_mht_mmcs.py")
print('Finished building MHT files.\n\n')

for i in range(0,12):
	print("choice:"+str(i))
	os.system("./prfm_eval_update_mht " + mht_file_array[i] + " " + str(i))
	time.sleep(1)	#sleep for 1s
	print('**********************************************************************')