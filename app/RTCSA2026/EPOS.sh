#!/bin/bash

set -x
#unset APPLICATION

env -i PATH="$PATH" bash -c 'cd EPOS && make veryclean'
env -i PATH="$PATH" bash -c 'cd EPOS && make APPLICATION=philosophers_dinner'

riscv64-linux-gnu-objcopy -O binary ./EPOS/img/philosophers_dinner.img /srv/tftp/EPOS.bin
