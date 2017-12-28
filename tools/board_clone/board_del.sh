#!/bin/bash

#You are running board_del.sh to delete a board automaticly.

#please input a board name like this: sp7730gga or sp8815ga

function select_product()
{
	echo "==================================================================="
	echo "please select product name:"
	echo "for example: sp8830ec or sp7715ga or sp7731gea or sp9630ea"
	echo "==================================================================="
	read PRODUCT_NAME
	#�����ɾboard��
}

function which_is_selected()
{
	echo "==================================================================="
	echo "you have selected :"
	echo "$PRODUCT_NAME"
	echo "==================================================================="
	#ȷ��������board��
}

function wrong_board_name()
{
	selected_board_path_scx15="device/sprd/scx15_$PRODUCT_NAME"
	selected_board_path_scx35="device/sprd/scx35_$PRODUCT_NAME"
	selected_board_path_scx35l="device/sprd/scx35l_$PRODUCT_NAME"
	temp=0
	if [ ! -d "$selected_board_path_scx15" ]&&[ ! -d "$selected_board_path_scx35" ]&&[ ! -d "$selected_board_path_scx35l" ];then
		echo "selected board not exist,please check againxxx!!!!!"
		exit 0
	fi
	if [  -d $selected_board_path_scx15 ];then
		temp=temp+1
	fi
	if [  -d "$selected_board_path_scx35" ];then
		temp=temp+1
	fi
	if [  -d "$selected_board_path_scx35l" ];then
		temp=temp+1
		echo scx5l
	fi
	if [ $temp -qe 0 ];then
		echo "selected board not exist,please check againxxx!!!!!"
		exit 0
	fi
	if [ $temp -gt 1 ];then
		echo "have two board,conflict,so exit!!!!!"
		exit 0
	fi
}	
	
function board_del_from_device()
{
	DEVICE_PATH_scx15="device/sprd/scx15""_""$PRODUCT_NAME"
	DEVICE_PATH_scx35="device/sprd/scx35""_""$PRODUCT_NAME"
	DEVICE_PATH_scx35l="device/sprd/scx35l""_""$PRODUCT_NAME"
	if [ ! -d $DEVICE_PATH_scx15 ];then
		echo "scx15 platform do not have this board!"
		else
		echo "$DEVICE_PATH_scx15  exist!"
		echo -e "del_product_device begin......"
		rm -rf $DEVICE_PATH_scx15
		#ɾ��device/sprd�����board��Ŀ¼�ļ��У�dolphin���
		echo "======del_product_device end======"
	fi
	if [ ! -d $DEVICE_PATH_scx35 ];then
		echo "scx35 platform do not have this board!"
		else
		echo "$DEVICE_PATH_scx35 exist!"
		echo -e "del_product_device begin......"
		rm -rf $DEVICE_PATH_scx35
		#ɾ��device/sprd�����board��Ŀ¼�ļ��У�shark��tshark���
		echo "======del_product_device end======"
	fi
	if [ ! -d $DEVICE_PATH_scx35l ];then
		echo "scx35l platform do not have this board!"
		else
		echo "$DEVICE_PATH_scx35l exist!"
		echo -e "del_product_device begin......"
		rm -rf $DEVICE_PATH_scx35l
		#ɾ��device/sprd�����board��Ŀ¼�ļ��У�sharkl
		echo "======del_product_device end======"
	fi
}

function board_del_from_uboot()	
{
	UBOOT_BOARD_PATH="u-boot/board/spreadtrum/""$PRODUCT_NAME"
	if [ ! -d $UBOOT_BOARD_PATH ];then
		echo "uboot do not have this board!"
		else 
		echo "$UBOOT_BOARD_PATH  exist in uboot!"
		echo -e "del_product_uboot_board_file begin......"
		rm -rf $UBOOT_BOARD_PATH
		#ɾ��uboot�����board��Ŀ¼�ļ���
	fi
	UBOOT_CONFIG_FILE="u-boot/include/configs/""$PRODUCT_NAME"".h"
	if [ ! -f $UBOOT_CONFIG_FILE ];then
		echo "uboot do not have this UBOOT_CONFIG_FILE!"
		else
		echo "$UBOOT_CONFIG_FILE  exist in uboot!"
		echo -e "del_product_uboot_config_file begin......"
		rm $UBOOT_CONFIG_FILE	
		echo "======del_product_uboot end====="
		#ɾ��uboot�����board��ͷ�ļ�
	fi
	#sed -i '/'"$PRODUCT_NAME"'_config/d' u-boot/Makefile
	#sed -i '/ '"$PRODUCT_NAME"' /d' u-boot/Makefile
	
	sed -i '/'"$PRODUCT_NAME"'_config/,/_config/{//!d}' u-boot/Makefile  #ɾ��ָ��һ���ı�����ʼ�кͽ����г���
	sed -i '/'"$PRODUCT_NAME"'_config/d' u-boot/Makefile
	
	PRODUCT_NAME_C=`tr '[a-z]' '[A-Z]' <<<"$PRODUCT_NAME"`
	DEFINED_BOARD_MACRO_04=" || (defined CONFIG_MACH_""$PRODUCT_NAME_C"")"
	
	DEFINED_BOARD_MACRO_03="defined(CONFIG_""$PRODUCT_NAME_C"")"

	sed -i 's/'"$DEFINED_BOARD_MACRO_04"'//' `grep $DEFINED_BOARD_MACRO_04 -rl --exclude-dir=".git" u-boot`	
	echo "$DEFINED_BOARD_MACRO_04"
	sed -i 's/ || '"$DEFINED_BOARD_MACRO_03"'//' `grep $DEFINED_BOARD_MACRO_03 -rl --exclude-dir=".git" u-boot`	
	echo "$DEFINED_BOARD_MACRO_03"
}

function board_del_from_chipram()	
{	
	CHIPRAM_CONFIG_FILE="chipram/include/configs/""$PRODUCT_NAME"".h"
	if [ ! -f $CHIPRAM_CONFIG_FILE ];then
		echo "CHIPRAM do not have this CHIPRAM_CONFIG_FILE!"
		else 
		echo "$CHIPRAM_CONFIG_FILE  exist in chipram!"
		echo -e "del_product_chipram_config_file begin......"
		rm $CHIPRAM_CONFIG_FILE
		#ɾ��chipram�����board��ͷ�ļ�
		echo "======del_product_chipram end====="	
	fi
	#sed -i '/'"$PRODUCT_NAME"'_config/d' chipram/Makefile
	#sed -i '/ '"$PRODUCT_NAME"' /d' chipram/Makefile
	
	sed -i '/'"$PRODUCT_NAME"'_config/,/_config/{//!d}' chipram/Makefile
	sed -i '/'"$PRODUCT_NAME"'_config/d' chipram/Makefile
	
	PRODUCT_NAME_C=`tr '[a-z]' '[A-Z]' <<<"$PRODUCT_NAME"`
	DEFINED_BOARD_MACRO_04=" || (defined CONFIG_MACH_""$PRODUCT_NAME_C"")"
	DEFINED_BOARD_MACRO_03="defined(CONFIG_""$PRODUCT_NAME_C"")"

	sed -i 's/'"$DEFINED_BOARD_MACRO_04"'//' `grep $DEFINED_BOARD_MACRO_04 -rl chipram`
	echo "$DEFINED_BOARD_MACRO_04"
	sed -i 's/ || '"$DEFINED_BOARD_MACRO_03"'//' `grep $DEFINED_BOARD_MACRO_03 -rl --exclude-dir=".git" chipram`
	echo "$DEFINED_BOARD_MACRO_03"
}

function board_del_from_kernel()	
{
	KERNEL_DECONFIG_FILE_DT="kernel/arch/arm/configs/""$PRODUCT_NAME""_dt_defconfig"
	KERNEL_DECONFIG_FILE_DT2="kernel/arch/arm/configs/""$PRODUCT_NAME""-dt_defconfig"
	KERNEL_DECONFIG_FILE_NATIVE="kernel/arch/arm/configs/""$PRODUCT_NAME""-native_defconfig"
	KERNEL_DECONFIG_FILE="kernel/arch/arm/configs/""$PRODUCT_NAME""_defconfig"
	if [ ! -f $KERNEL_DECONFIG_FILE_DT ];then
		echo "kernel do not have this KERNEL_DECONFIG_FILE_DT!"
		else 
		echo "$KERNEL_DECONFIG_FILE_DT  exist in kernel!"
		echo -e "del_product_kernel_deconfig_file_dt ......"
		rm $KERNEL_DECONFIG_FILE_DT
		#ɾ��kernel�����dt�汾defconfig�ļ�
	fi
	if [ ! -f $KERNEL_DECONFIG_FILE_DT2 ];then
		echo "kernel do not have this KERNEL_DECONFIG_FILE-DT!"
		else 
		echo "$KERNEL_DECONFIG_FILE-DT  exist in kernel!"
		echo -e "del_product_kernel_deconfig_file-dt ......"
		rm $KERNEL_DECONFIG_FILE_DT2
		#ɾ��kernel�����dt�汾defconfig�ļ�
	fi
	if [ ! -f $KERNEL_DECONFIG_FILE_NATIVE ];then
		echo "kernel do not have this KERNEL_DECONFIG_FILE_NATIVE!"
		else 
		echo "$KERNEL_DECONFIG_FILE_NATIVE  exist in kernel!"
		echo -e "del_product_kernel_deconfig_file_native ......"
		rm $KERNEL_DECONFIG_FILE_NATIVE
		#ɾ��kernel�����native�汾defconfig�ļ�
	fi
	if [ ! -f $KERNEL_DECONFIG_FILE ];then
		echo "kernel do not have this KERNEL_DECONFIG_FILE!"
		else 
		echo "$KERNEL_DECONFIG_FILE  exist in kernel!"
		echo -e "del_product_kernel_deconfig_file ......"
		rm $KERNEL_DECONFIG_FILE
		#ɾ��kernel�����defconfig�ļ�
	fi
	KERNEL_SCX15_DTS_FILE="kernel/arch/arm/boot/dts/""sprd-scx15_""$PRODUCT_NAME"".dts"
	KERNEL_SCX35_DTS_FILE="kernel/arch/arm/boot/dts/""sprd-scx35_""$PRODUCT_NAME"".dts"
	KERNEL_SCX35L_DTS_FILE="kernel/arch/arm/boot/dts/""sprd-scx35l_""$PRODUCT_NAME"".dts"
	if [ ! -f $KERNEL_SCX15_DTS_FILE ];then
		echo "kernel do not have this KERNEL_SCX15_DTS_FILE!"
		else 
		echo "$KERNEL_SCX15_DTS_FILE  exist in kernel!"
		echo -e "del_product_kernel_SCX15_DTS_FILE ......"
		rm $KERNEL_SCX15_DTS_FILE
		#ɾ��kernel����board������dts�ļ�,dolphin���
	fi
	if [ ! -f $KERNEL_SCX35_DTS_FILE ];then
		echo "kernel do not have this KERNEL_SCX35_DTS_FILE!"
		else 
		echo "$KERNEL_SCX35_DTS_FILE  exist in kernel!"
		echo -e "del_product_kernel_SCX35_DTS_FILE ......"
		rm $KERNEL_SCX35_DTS_FILE
		#ɾ��kernel����board������dts�ļ�,shark��tshark���
	fi
	if [ ! -f $KERNEL_SCX35L_DTS_FILE ];then
		echo "kernel do not have this KERNEL_SCX35L_DTS_FILE!"
		else 
		echo "$KERNEL_SCX35_DTS_FILE  exist in kernel!"
		echo -e "KERNEL_SCX35L_DTS_FILE ......"
		rm $KERNEL_SCX35L_DTS_FILE
		#ɾ��kernel����board������dts�ļ�,sharkl
	fi
	KERNEL_BOARD_C="kernel/arch/arm/mach-sc/""board-""$PRODUCT_NAME"".c"
	if [ ! -f $KERNEL_BOARD_C ];then
		echo "KERNEL do not have this KERNEL_BOARD_C file!"
		else 
		echo "$KERNEL_BOARD_C  exist in kernel!"
		echo -e "del_product_kernel_KERNEL_BOARD_C file begin......"
		rm $KERNEL_BOARD_C
		#ɾ��kernel����board-sp****.c�ļ�
	fi
	KERNEL_BOARD_H="kernel/arch/arm/mach-sc/include/mach/""__board-""$PRODUCT_NAME"".h"
	if [ ! -f $KERNEL_BOARD_H ];then
		echo "KERNEL do not have this KERNEL_BOARD_H file!"
		else
		echo "$KERNEL_BOARD_H  exist in kernel!"
		echo -e "del $KERNEL_BOARD_H  begin......"
		rm $KERNEL_BOARD_H
		#ɾ��kernel����__board-sp****.h�ļ�
	fi

	PRODUCT_NAME_C=`tr '[a-z]' '[A-Z]' <<<"$PRODUCT_NAME"`
	#��дboard��

	DEFINED_BOARD_MACRO_04=" || (defined CONFIG_MACH_""$PRODUCT_NAME_C"")"
	echo $DEFINED_BOARD_MACRO_04
	sed -i 's/'"$DEFINED_BOARD_MACRO_04"'//' `grep $DEFINED_BOARD_MACRO_04 -rl --exclude-dir=".git" kernel`

	DEFINED_BOARD_MACRO_03="defined(CONFIG_MACH_""$PRODUCT_NAME_C"")"
	#��дboard���ĺ궨����
	echo "$DEFINED_BOARD_MACRO_03"
	sed -i 's/ || '"$DEFINED_BOARD_MACRO_03"'//' `grep $DEFINED_BOARD_MACRO_04 -rl --exclude-dir=".git" kernel`
	#ɾ��board.h�ļ��У���дboard���ĺ궨�������ַ���

	declare -i line_index  #���ڼ����к�
	#IF_DEF_PRODUCT_NAME="#ifdef CONFIG_MACH_""$PRODUCT_NAME_C"
	TEMP="CONFIG_MACH_""$PRODUCT_NAME_C"
	line_index=`sed -n "/$TEMP$""/=" kernel/arch/arm/mach-sc/include/mach/board.h`  #�����ȣ�����sp7730ec��sp7730ectrisim����sp7730ecopenphone����
	echo $line_index
	if [ $line_index = 0 ];then
		echo "something error!!!"
	else
		line_index=line_index-1
		echo $line_index
		sed -i "$line_index""d" kernel/arch/arm/mach-sc/include/mach/board.h
		sed -i '/'"CONFIG_MACH_$PRODUCT_NAME_C"'$/,/#endif/d' kernel/arch/arm/mach-sc/include/mach/board.h
		#ɾ��board.h�ļ��У���дboard����ʼ������������һ��#endif�������м�ȫ������
		#����ȫƥ��á���дboard���������硰��дboard��_LC���汾��ɾ
	fi

	PRODUCT_NAME_C_DTS="(CONFIG_MACH_""$PRODUCT_NAME_C"")"
	sed -i '/'"$PRODUCT_NAME_C_DTS"'/d' kernel/arch/arm/boot/dts/Makefile
	#ɾ��kernel/arch/arm/boot/dts/Makefile�У���дboard���ĺ궨�������е�һ����
	BOARD_BASED="board"
	MACH_PRODUCT_NAME_C="MACH_""&PRODUCT_NAME_C"

	TEMP="MACH_""$PRODUCT_NAME_C"
	line_index=`sed -n "/$TEMP$""/=" kernel/arch/arm/mach-sc/Kconfig`
	if [ $line_index = 0 ];then
		echo "something error!!!"
	else
		line_index=line_index-1
		echo $line_index
		sed -i "$line_index""d" kernel/arch/arm/mach-sc/Kconfig
		sed -i '/MACH_'"$PRODUCT_NAME_C"'$/,/'"$BOARD_BASED"'/d' kernel/arch/arm/mach-sc/Kconfig
		#ɾ��kconfig�У���дboard�������һ�Σ�����ɾ������ȫƥ��
	fi

	sed -i '/board-'"$PRODUCT_NAME"'.o/d' kernel/arch/arm/mach-sc/Makefile
	#ɾ��kernel/arch/arm/mach-sc/Makefile��,sp****.o������
}


if [ ! -d "./kernel" ];then
	echo "board_del.sh maybe is not in correct dir,please put it to android top dir!!"
	exit 0
fi
if [ ! -d "./u-boot" ];then
	echo "board_del.sh maybe is not in correct dir,please put it to android top dir!!"
	exit 0
fi
if [ ! -d "./chipram" ];then
	echo "board_del.sh maybe is not in correct dir,please put it to android top dir!!"
	exit 0
fi
if [ ! -d "./device/sprd" ];then
	echo "board_del.sh maybe is not in correct dir,please put it to android top dir!!"
	exit 0
fi

PRODUCT_NAME=

select_product

which_is_selected

wrong_board_name

board_del_from_device

board_del_from_uboot

board_del_from_chipram

board_del_from_kernel
