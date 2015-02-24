# The LST var holds a list of gpio numbers for the port pins (see p1.txt)

LST=""

COMMPORT="/dev/ttyS4"
SYS_GPIO="/sys/class/gpio"

function setportpins {
  LST=`awk '$0!~/^#/ { printf"%s ",$3 }' $1.txt`
}

function flushcomport {
  while read -t 1 LINE
  do
    echo "flush line " $LINE
  done < $COMMPORT
}

function initgpio {
for PIN in $LST
do
  if [ $PIN != "-" ]
  then
    if [ ! -d $SYS_GPIO/gpio$PIN ]
    then
      echo $PIN > $SYS_GPIO/export 2> /dev/null
    fi
  fi
done
}

function initremotegpio {
  for PIN in $LST
  do
    if [ $PIN != "-" ]
    then
      echo "echo $PIN > $SYS_GPIO/export" > $COMMPORT
    fi
  done
}

function setremotedir {
  echo "echo $2 > $SYS_GPIO/gpio$1/direction" > $COMMPORT
}

function setdir {
  echo $2 >$SYS_GPIO/gpio$1/direction
}

function setval {
  if [ $2 != "-" ]
  then
    if [ -e $SYS_GPIO/gpio$1 ]
    then
      echo $2 > $SYS_GPIO/gpio$1/value
    fi
  fi
}

function setremoteval {
  if [ $2 != "-" ]
  then
      echo "echo $2 > $SYS_GPIO/gpio$1/value" > $COMMPORT
  fi
}

function getval {
  GETVAL=`cat $SYS_GPIO/gpio$1/value`
}

function getremoteval {
  echo head -c 1 $SYS_GPIO/gpio$1/value > $COMMPORT
  read -n 1 -t 5 -e GETREMOTEVAL < /dev/ttyS4
}

function printport {
  for PIN in $LST
  do
    if [ $PIN != "-" ]
    then 
      if [ -e $SYS_GPIO/gpio$PIN ]
      then
        getval $PIN
        echo -n $GETVAL
      else
        echo -n "x"
      fi
    else
      echo -n "-"
    fi
  done
  echo ""
}


function printremoteport {
for PIN in $LST
  do
    if [ $PIN != "-" ]
    then 
      if [ -e $SYS_GPIO/gpio$PIN ]
      then
        getremoteval $PIN
        echo -n $GETREMOTEVAL
      else
        echo -n "x" # > $COMMPORT
      fi
    else
      echo -n "-" # > $COMMPORT
    fi
  done
  echo "" # > $COMMPORT
}

function setportval {
  for PIN in $LST
  do
    if [ $PIN != "-" ]
    then 
      if [ -e $SYS_GPIO/gpio$PIN ]
      then
        setval $PIN $1
      fi
    fi
  done
}

function setportdir {
  for PIN in $LST
  do
    if [ $PIN != "-" ]
    then 
      if [ -e $SYS_GPIO/gpio$PIN ]
      then
        setdir $PIN $1
      fi
    fi
  done
}

function setremoteportdir {
  for PIN in $LST
  do
    if [ $PIN != "-" ]
    then 
      setremotedir $PIN $1
    fi
  done
}

function playtestfile {
  pinidx=($LST)
  # use sed to put a space (delimiter) between each char in a tmp file 
  sed 's/./& /g' < $1 > $1_tmp

  while read line
  do
    local pos=0
    for char in $line
    do
      setval ${pinidx[$pos]} $char
      ((pos++)) #  same as but much faster  pos=`expr $pos + 1`
    done
  printremoteport
  done < $1_tmp
  rm $1_tmp
}

function playtestfileremote {
  pinidx=($LST)
  # use sed to put a space (delimiter) between each char in a tmp file 
  sed 's/./& /g' < $1 > $1_tmp

  while read line
  do
    local pos=0
    for char in $line
    do
      setremoteval ${pinidx[$pos]} $char
      ((pos++)) #  same as but much faster  pos=`expr $pos + 1`
    done
  printport
  done < $1_tmp
  rm $1_tmp
}

function setupgpio {
  # init local port, briefly make port out to take control in kernel
  initgpio
  setportdir out
  sleep 0.5
  setportdir in

  # init remote port, briefly make port out to take control in kernel
  initremotegpio
  setremoteportdir out
  sleep 0.5
  setremoteportdir in

  #flush error messages form com port
  flushcomport
}

function readport {
  #set local to write and send test file
  setportpins $1
  setportdir out
  setremoteportdir in
  sleep 0.5
  playtestfile $1"testsend.lst"
}

function writeport {
  #set remote to write and send test file
  setportpins $1
  setremoteportdir out
  setportdir in
  sleep 0.5
  playtestfileremote $1"testrecv.lst"
}
