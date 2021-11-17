#!/bin/bash
cd ../

# Small buffer
seed=2
dirnum=1
target=$(($seed+5))
while [ $seed -le $target ]
do
  ./waf --run "scratch/test-mmw --ccProt=TcpBbr --seed=$seed --lteBuff=2000000" 
  cd automate
  python processdata.py
  cd ..
  mv traces/test traces/$dirnum
  dirnum=$(($dirnum+1))

  ./waf --run "scratch/test-mmw --ccProt=TcpSiad --seed=$seed --lteBuff=2000000" 
  cd automate
  python processdata.py
  cd ..
  mv traces/test traces/$dirnum
  dirnum=$(($dirnum+1))
  echo "seed=$seed run finished"
  seed=$(($seed+1))
done

# Medium buffer
seed=2
target=$(($seed+5))
while [ $seed -le $target ]
do
  ./waf --run "scratch/test-mmw --ccProt=TcpBbr --seed=$seed --lteBuff=7000000" 
  cd automate
  python processdata.py
  cd ..
  mv traces/test traces/$dirnum
  dirnum=$(($dirnum+1))

  ./waf --run "scratch/test-mmw --ccProt=TcpSiad --seed=$seed --lteBuff=7000000" 
  cd automate
  python processdata.py
  cd ..
  mv traces/test traces/$dirnum
  dirnum=$(($dirnum+1))
  echo "seed=$seed run finished"
  seed=$(($seed+1))
done

# Large buffer
seed=2
target=$(($seed+5))
while [ $seed -le $target ]
do
  ./waf --run "scratch/test-mmw --ccProt=TcpBbr --seed=$seed --lteBuff=20000000" 
  cd automate
  python processdata.py
  cd ..
  mv traces/test traces/$dirnum
  dirnum=$(($dirnum+1))

  ./waf --run "scratch/test-mmw --ccProt=TcpSiad --seed=$seed --lteBuff=20000000" 
  cd automate
  python processdata.py
  cd ..
  mv traces/test traces/$dirnum
  dirnum=$(($dirnum+1))
  echo "seed=$seed run finished"
  seed=$(($seed+1))
done

python plot.py
