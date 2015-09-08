count=16

j=$count
echo "#define __rseqn( s ) \\"
while [ $j -gt 0 ]; do
  echo " _TO_GPIO_##s##$j, a2$j, a3$j, a4$j, a5$j, a6$j, a7$j, a8$j, \\"
  j=`expr $j - 1`
done
echo " _TO_GPIO_##s##0"

j=0
echo "#define __argsn( \\"
while [ $j -lt $count ]; do
  echo " a1$j, a2$j, a3$j, a4$j, a5$j, a6$j, a7$j, a8$j, \\"
  j=`expr $j + 1`
done
echo " n, ... ) n"
echo "#define __funcn(...) __argsn( __VA_ARGS__ )"
echo

for RNAME in MODER OTYPER OSPEEDR PUPDR ODR AFRL AFRH; do
  echo "#define TO_GPIO_$RNAME(DEST_PORT, ...) __funcn( __VA_ARGS__,__rseqn($RNAME))(DEST_PORT,__VA_ARGS__)"
  echo
  j=1
  while [ $j -le $count ]; do
    echo "#define _TO_GPIO_$RNAME$j(DEST_PORT, \\"
    i=1
    while [ $i -lt $j ]; do
      echo " PORT$i, PIN$i, MODE$i, SPEED$i, OUT_MODE$i, PULL$i, AF$i, SET$i, \\"
      i=`expr $i + 1`
    done
    echo " PORT$j, PIN$j, MODE$j, SPEED$j, OUT_MODE$j, PULL$j, AF$j, SET$j) \\"
    i=1
    while [ $i -lt $j ]; do
      echo " _TO_GPIO_$RNAME(DEST_PORT, PORT$i, PIN$i, MODE$i, SPEED$i, OUT_MODE$i, PULL$i, AF$i, SET$i)|\\"
      i=`expr $i + 1`
    done
    echo " _TO_GPIO_$RNAME(DEST_PORT, PORT$j, PIN$j, MODE$j, SPEED$j, OUT_MODE$j, PULL$j, AF$j, SET$j)"
    echo
    j=`expr $j + 1`
  done
done