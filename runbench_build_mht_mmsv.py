import os
import time

for i in range(0,12):
	for j in range(0,100):
		print("choice:"+str(i))
		os.system("./prfm_eval_build_mht " + str(i))
		time.sleep(3)
	print('**********************************************************************')
