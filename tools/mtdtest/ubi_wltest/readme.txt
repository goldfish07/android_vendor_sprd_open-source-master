//

��/дͬһ�߼���eb�ĸ��ؾ������

1.������԰汾���������Է���
��Ҫʹ��nand�汾������ʱ���Ӳ��Է�������Ӧ�����ļ���Ҫ�޸ģ��磺�ļ�sp7730gga.h������
#define MTDPARTS_DEFAULT "mtdparts=sprd-nand:256k(spl),768k(2ndbl),512k(kpanic),-(ubipac)"
��Ϊ
#define MTDPARTS_DEFAULT "mtdparts=sprd-nand:256k(spl),768k(2ndbl),512k(kpanic),32m(testubi),-(ubipac)"

make bootloader -j4�����������ļ� fdl2.bin, u-boot.bin�����滻scx35_sp7730ggacuccspecBplus_UUI_dt-userdebug-native.pac�ж�Ӧ�ļ�

2.����Դ�ļ������ļ�ubiwltest

3.���������ļ�tang.sh

stop
/data/ubiwltest 8
/data/ubiwltest 1
/data/ubiwltest 4
/data/ubiwltest 6 4
start

���������в���������Ϊ
/data/ubiwltest 6 4(8/64/128/1024/4096/8192)

4.�����������ļ�(windowsϵͳ)testubi.bat

adb devices

adb root
adb remount

adb push ubiwltest ./data
adb shell chmod 755 ./data/ubiwltest

adb push tang.sh ./system/bin
adb shell chmod 777 ./system/bin/tang.sh

adb shell ./system/bin/tang.sh

adb pull ./data/mtd_test.txt
adb pull ./data/test_wl.txt
adb pull ./data/test_wl_diff.txt

5.����ִ�н�����ļ�mtd_test.txt
attach finish ubi_num = 1, s_leb_size=3e000
do_test_wl start
do_test_wl finish! num_of_times=4096.
pebnums = 128, mean = 8192, deviation=0.067628.
min_ec = 5104, max_ec = 10071
������deviationԽСԽ��
