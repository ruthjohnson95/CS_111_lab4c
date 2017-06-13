#!/bin/sh

gcc -lmraa -lm  -o lab4b lab4b.c

let errors=0

./lab4b --period=2 --scale=C --log="LOGFILE" <<-EOF
SCALE=F
SCALE=F
SCALE=C
SCALE=C
PERIOD=1
PERIOD=10
START
START
STOP
STOP
OFF
EOF

ret=$?

if [ $ret -ne 0 ]
then
  let errors+=1
  echo "...ERROR: failed smoke test"
fi

if [ ! -s LOGFILE ]
then
  let errors+=1
  echo "...ERROR: did not create a log file"
else 
  rm LOGFILE 
fi

if [ $errors -eq 0 ]; then
  echo "...PASSED SMOKE TEST"
fi


