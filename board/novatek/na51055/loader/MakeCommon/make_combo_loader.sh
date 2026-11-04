#!/bin/bash
MODEL_BUILD_LIST=("lunch rtos cfg_TEST_REALCHIP gcc-linaro-6.4.1-2018.05-x86_64_arm-eabi"
			"lunch rtos cfg_TEST_RTOS_RUN_CORE2 gcc-linaro-6.4.1-2018.05-x86_64_arm-eabi")

function make_NT98525_loader_only(){
  cp ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h ../LibExt/LIBExt_src/Ctrl_Flow/bl_func_tmp.h
  cp ../Project/Model/ModelConfig_EMU_EVB_528.txt ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt
  cp ../Project/Model/ModelConfig_EMU_EVB.txt ../Project/Model/ModelConfig_EMU_EVB_tmp.txt
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_52x/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  make clean "MODEL=EMU_EVB"
  make clean "MODEL=EMU_EVB_528"
  make release "MODEL=EMU_EVB"
  if [ -d "../output" ]; then
	rm -rf ../output
	mkdir -p ../output
  else
	mkdir -p ../output
  fi
  find ../Project/Model/ -name LD*.bin -exec cp {} ../output \;
  cp ../LibExt/LIBExt_src/Ctrl_Flow/bl_func_tmp.h ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  cp ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt ../Project/Model/ModelConfig_EMU_EVB_528.txt
  cp ../Project/Model/ModelConfig_EMU_EVB_tmp.txt ../Project/Model/ModelConfig_EMU_EVB.txt
  #cp ../Project/Model/Loader525_Data/Release/LD98525A.bin ../output
}

function make_NT98528_loader_only(){
  cp ../Project/Model/ModelConfig_EMU_EVB_528.txt ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt
  cp ../Project/Model/ModelConfig_EMU_EVB.txt ../Project/Model/ModelConfig_EMU_EVB_tmp.txt
  cp ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h ../LibExt/LIBExt_src/Ctrl_Flow/bl_func_tmp.h
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_528/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  make clean "MODEL=EMU_EVB"
  make clean "MODEL=EMU_EVB_528"
  make release "MODEL=EMU_EVB_528"
  if [ -d "../output" ]; then
	rm -rf ../output
	mkdir -p ../output
  else
	mkdir -p ../output
  fi
  find ../Project/Model/ -name LD*.bin -exec cp {} ../output \;
  cp ../LibExt/LIBExt_src/Ctrl_Flow/bl_func_tmp.h ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  cp ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt ../Project/Model/ModelConfig_EMU_EVB_528.txt
  cp ../Project/Model/ModelConfig_EMU_EVB_tmp.txt ../Project/Model/ModelConfig_EMU_EVB.txt
  #cp ../Project/Model/Loader528_Data/Release/LD98528A.bin ../output
}

function make_NT9852x_combo_loader(){
  if [ -d "../output" ]; then
	rm -rf ../output
	mkdir -p ../output
  else
	mkdir -p ../output
  fi
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     COMBINATION_52x/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  make clean "MODEL=EMU_EVB"
  make release "MODEL=EMU_EVB" "BIN_NAME=LDCOMBOA" "52x_528_COMBO=ON"
  find ../Project/Model/ -name LD*.bin -exec cp {} ../output/LD9852XA.bin \;
  #cp ../Project/Model/Loader525_Data/Release/LDCOMBOA.bin ../output/LD98525A.bin
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     COMBINATION_528/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  make clean
  make release "MODEL=EMU_EVB_528" "BIN_NAME=LDCOMBOA"
  find ../Project/Model/ -name LD*.bin -exec cp {} ../output/LD98528A.bin \;
  #cp ../Project/Model/Loader528_Data/Release/LDCOMBOA.bin ../output/LD98528A.bin
  cat ../output/LD9852XA.bin ../output/LD98528A.bin > ../output/LDCOMBOA.bin
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_52x/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
}

function make_NT98525_eth_write_loader(){
  cp ../Project/Model/ModelConfig_EMU_EVB.txt ../Project/Model/ModelConfig_EMU_EVB_tmp.txt
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_52x/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  sed -i 's/^STORAGEEXT =[ \t].*/STORAGEEXT = Eth/g'  ../Project/Model/ModelConfig_EMU_EVB.txt
  make clean "MODEL=EMU_EVB"
  make clean "MODEL=EMU_EVB_528"
  make release "MODEL=EMU_EVB"
  if [ -d "../output" ]; then
	rm -rf ../output
	mkdir -p ../output
  else
	mkdir -p ../output
  fi
  find ../Project/Model/ -name loader.bin -exec cp {} ../output \;
  cp ../Project/Model/ModelConfig_EMU_EVB_tmp.txt ../Project/Model/ModelConfig_EMU_EVB.txt
  rm -rf ../Project/Model/ModelConfig_EMU_EVB_tmp.txt
  #cp ../Project/Model/Loader525_Data/Release/LD98525A.bin ../output
}

function make_NT98528_eth_write_loader(){
  cp ../Project/Model/ModelConfig_EMU_EVB_528.txt ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_528/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  sed -i 's/^STORAGEEXT =[ \t].*/STORAGEEXT = Eth/g'  ../Project/Model/ModelConfig_EMU_EVB_528.txt
  make clean "MODEL=EMU_EVB"
  make clean "MODEL=EMU_EVB_528"
  make release "MODEL=EMU_EVB_528"
  if [ -d "../output" ]; then
    rm -rf ../output
    mkdir -p ../output
  else
    mkdir -p ../output
  fi
  find ../Project/Model/ -name loader.bin -exec cp {} ../output/loader.bin \;
  cp ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt ../Project/Model/ModelConfig_EMU_EVB_528.txt
  rm -rf ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt
}

function make_NT98525_usb_write_loader(){
  cp ../Project/Model/ModelConfig_EMU_EVB.txt ../Project/Model/ModelConfig_EMU_EVB_tmp.txt
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_52x/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  sed -i 's/^STORAGEEXT =[ \t].*/STORAGEEXT = Usb/g'  ../Project/Model/ModelConfig_EMU_EVB.txt
  make clean "MODEL=EMU_EVB"
  make clean "MODEL=EMU_EVB_528"
  make release "MODEL=EMU_EVB"
  if [ -d "../output" ]; then
	rm -rf ../output
	mkdir -p ../output
  else
	mkdir -p ../output
  fi
  find ../Project/Model/ -name LD98525A.bin -exec cp {} ../output/Wrldr525.bin \;
  #cp ../Project/Model/Loader525_Data/Release/LD98525A.bin ../output
  cp ../Project/Model/ModelConfig_EMU_EVB_tmp.txt ../Project/Model/ModelConfig_EMU_EVB.txt
  rm -rf ../Project/Model/ModelConfig_EMU_EVB_tmp.txt
}

function make_NT98528_usb_write_loader(){
  cp ../Project/Model/ModelConfig_EMU_EVB_528.txt ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_528/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  sed -i 's/^STORAGEEXT =[ \t].*/STORAGEEXT = Usb/g'  ../Project/Model/ModelConfig_EMU_EVB_528.txt
  make clean "MODEL=EMU_EVB"
  make clean "MODEL=EMU_EVB_528"
  make release "MODEL=EMU_EVB_528"
  if [ -d "../output" ]; then
    rm -rf ../output
    mkdir -p ../output
  else
    mkdir -p ../output
  fi
  find ../Project/Model/ -name LD98528A.bin -exec cp {} ../output/Wrldr528.bin \;
  cp ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt ../Project/Model/ModelConfig_EMU_EVB_528.txt
  rm -rf ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt
}

make_NT98528_sqa_fastboot_test_nand(){
  cp ../Project/Model/ModelConfig_EMU_EVB_528.txt ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt
  cp ../Project/Model/ModelConfig_EMU_EVB.txt ../Project/Model/ModelConfig_EMU_EVB_tmp.txt
  cp ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h ../LibExt/LIBExt_src/Ctrl_Flow/bl_func_tmp.h
  cp ../Project/Model/Src/prj_main.h ../Project/Model/Src/prj_main_tmp.h

  sed -i 's/^SDRAM2_SIZE =[ \t].*/SDRAM2_SIZE = 0/g'  ../Project/Model/ModelConfig_EMU_EVB_528.txt
  sed -i 's/^#define DEBUG_MSG[ \t].*/#define DEBUG_MSG                   (DISABLE)/g'  ../Project/Model/Src/prj_main.h
  sed -i 's/^#define LOADER_TYPE[ \t].*/#define LOADER_TYPE                     STAND_ALONE_LOADER_528/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  sed -i 's/^[ \t]\/\/strg_obj->flash_setFrequency(96);*/strg_obj->flash_setFrequency(96);/g'  ../Project/Model/Src/prj_main.c
  make clean "MODEL=EMU_EVB"
  make clean "MODEL=EMU_EVB_528"
  make release "MODEL=EMU_EVB_528"
  if [ -d "../output" ]; then
        rm -rf ../output
        mkdir -p ../output
  else
        mkdir -p ../output
  fi
  find ../Project/Model/ -name LD*.bin -exec cp {} ../output \;
  cp ../LibExt/LIBExt_src/Ctrl_Flow/bl_func_tmp.h ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
  cp ../Project/Model/ModelConfig_EMU_EVB_528_tmp.txt ../Project/Model/ModelConfig_EMU_EVB_528.txt
  cp ../Project/Model/ModelConfig_EMU_EVB_tmp.txt ../Project/Model/ModelConfig_EMU_EVB.txt
  cp ../Project/Model/Src/prj_main_tmp.h ../Project/Model/Src/prj_main.h  
  #cp ../Project/Model/Loader528_Data/Release/LD98528A.bin ../output
}

echo -e "Please select NMAKE environment:"
echo -e "1. make NT9852x standalone loader(MODEL = EMU_EVB)"
echo -e "2. make NT98528/NT98529 standalone loader(MODEL = EMU_EVB_528)"
echo -e "3. make NT9852x & NT98528 combination loader"
echo -e "4. make NT9852x Eth write loader(MODEL = EMU_EVB)"
echo -e "5. make NT98528/NT98529 Eth write loader(MODEL = EMU_EVB_528)"
echo -e "6. make NT9852x Usb write loader(MODEL = EMU_EVB)"
echo -e "7. make NT98528/NT98529 Usb write loader(MODEL = EMU_EVB_528)"
echo -e "-------------------------------------------------------"
echo -e "8. make NT9852x (tee) & NT98528 (tee) combination loader"
echo -e "-------------------------------------------------------"
echo -e "z. make NT9852x MT version loader(in house usage)"
echo -e "-------------------------------------------------------"
echo -e "s. make NT98528 for SQA fastboot test(internal usage)"
echo -e "-------------------------------------------------------"


read NMAKE_ENV
case ${NMAKE_ENV} in
  "1")
    make_NT98525_loader_only;
    ;;
  "2")
    make_NT98528_loader_only;
    ;;
  "3")
    make_NT9852x_combo_loader;
    ;;
  "4")
    make_NT98525_eth_write_loader;
    ;;
  "5")
    make_NT98528_eth_write_loader;
    ;;
  "6")
    make_NT98525_usb_write_loader;
    ;;
  "7")
    make_NT98528_usb_write_loader;
    ;;
  "8")
    sed -i 's/^#define SDRAM_Start_FW[ \t].*/#define SDRAM_Start_FW              0x08000000/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
    make_NT9852x_combo_loader;
    sed -i 's/^#define SDRAM_Start_FW[ \t].*/#define SDRAM_Start_FW              0x02000000/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
    ;;
  "z")
    sed -i 's/^#define UITRON_FW[ \t].*/#define UITRON_FW                 ENABLE/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
    make_NT98525_loader_only;
    sed -i 's/^#define UITRON_FW[ \t].*/#define UITRON_FW                 DISABLE/g'  ../LibExt/LIBExt_src/Ctrl_Flow/bl_func.h
    ;;
  "s")
    make_NT98528_sqa_fastboot_test_nand;
    ;;

  *)
    echo -e "exit"
    ;;
esac



