cd ~/asst1/src/kern/conf
./config ASST1
cd ../compile/ASST1/
bmake depend
bmake WERROR=
bmake install
