import os
import time

os.system("make clean-mhtfile")
os.system("python runbench_build_mht_mmcs.py")
os.system("./prfm_eval_insertsp")
