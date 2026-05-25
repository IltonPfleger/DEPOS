#!/bin/bash

set -x
unset APPLICATION

env -i PATH="$PATH" bash -c 'cd EPOS && make veryclean'
env -i PATH="$PATH" bash -c 'cd EPOS && make APPLICATION=philosophers_dinner'

mv ./EPOS/img/philosophers_dinner.img /srv/tftp
