import os
import time

os.system("make clean-mhtfile_inscmn")
os.system("python runbench_build_mht_mmcs_inscmn.py")
os.system("./prfm_eval_insertcmn")
