BOARD_ID=`head -c8 /sys/devices/platform/jz4780-efuse/user_id`

OUTPUTFILE="ci20_gpiotest_"$BOARD_ID"_"`date +%H%M%S_%d%m%y`

echo "generating test report in $OUTPUTFILE"
echo "wait about 30 seconds"

echo "Test report for ci20 id $BOARD_ID generated on :" > $OUTPUTFILE
date >> $OUTPUTFILE

if [ "$1" == "-v" ]; then
  VERBOSE=true
fi
RES=0

source gpioport.inc

function readwriteport {
  readport $1 > $1"readout.res"
  if [[ -v VERBOSE ]]; then
    readport $1
  fi
  echo "diff between test pattern and received data for port read"
  echo +++++++++++
  diff $1"testsend.lst" $1"readout.res"
  ((RES+=$?))
  echo +++++++++++

  writeport $1 > $1"writeout.res"
  if [[ -v VERBOSE ]]; then
    writeport $1
  fi
  echo "diff between test pattern and received data for port write"
  echo +++++++++++
  diff $1"testrecv.lst" $1"writeout.res"
  ((RES+=$?))
  echo +++++++++++
}

setupgpio

#  Primry expansion header 
echo $'\nPrimry expansion header' >> $OUTPUTFILE

readwriteport p1 >> $OUTPUTFILE

#  Secondard expansion header 
echo $'\nSecondard expansion header' >> $OUTPUTFILE

readwriteport p2 >> $OUTPUTFILE

if [ $RES -gt 0 ]; then
  echo "TEST FAILED file diff returns "$RES  >> $OUTPUTFILE
  echo "compare *.lst and *.res files for more details"  >> $OUTPUTFILE
else
  echo "TEST PASSED all port reads/writes match"  >> $OUTPUTFILE
fi

cat $OUTPUTFILE

exit $RES
