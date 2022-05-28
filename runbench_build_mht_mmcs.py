import os
import time

for i in range(0,12):
	print("choice:"+str(i))
	os.system("./prfm_eval_build_mht_mmcs 0 " + str(i))
	time.sleep(1)	#sleep for 1s
	print('**********************************************************************')
