/**
    General Purpose I/O controller header file

    General Purpose I/O controller header file

    @file       GPIO.h
    @ingroup    mIIOGPIO
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _GPIO_H
#define _GPIO_H

#include "IOReg.h"

#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

/**
    @addtogroup mIDrvIO_GPIO
*/
//@{

/**
    GPIO direction

    GPIO direction definition for gpio_setDir() and gpio_getDir()
*/
typedef enum {
	GPIO_DIR_INPUT =            0,      ///< GPIO is input direction
	GPIO_DIR_OUTPUT =           1,      ///< GPIO is output direction

	ENUM_DUMMY4WORD(GPIO_DIR)
} GPIO_DIR;

/**
    GPIO interrupt type

    GPIO interrupt type definition for type argument of gpio_setIntTypePol()
*/
typedef enum {
	GPIO_INTTYPE_EDGE =         0,      ///< GPIO interrupt is edge trigger
	GPIO_INTTYPE_LEVEL =        1,      ///< GPIO interrupt is level trigger

	ENUM_DUMMY4WORD(GPIO_INTTYPE)
} GPIO_INTTYPE;

/**
    GPIO interrupt polarity

    GPIO interrupt polarity definition for pol argument of gpio_setIntTypePol()
*/
typedef enum {
	GPIO_INTPOL_POSHIGH =       0,      ///< GPIO interrupt polarity is \n
	///< - @b positvie edge for edge trigger
	///< - @b high level for level trigger
	GPIO_INTPOL_NEGLOW =        1,      ///< GPIO interrupt polarity is \n
	///< - @b negative edge for edge trigger
	///< - @b low level for level trigger
	GPIO_INTPOL_BOTHEDGE =      2,      ///< GPIO interrupt polarity is \n
	///< - @b both edge for edge trigger
	ENUM_DUMMY4WORD(GPIO_INTPOL)
} GPIO_INTPOL;



/**
    @name   GPIO pins ID

    GPIO pins ID definition

    For detail GPIO pin out, please refer to NT96520 data sheet.
*/
//@{
/*Storage GPIO - CGPIO*/
#define C_GPIO_0    0                       ///< C_GPIO[0]
#define C_GPIO_1    1                       ///< C_GPIO[1]
#define C_GPIO_2    2                       ///< C_GPIO[2]
#define C_GPIO_3    3                       ///< C_GPIO[3]
#define C_GPIO_4    4                       ///< C_GPIO[4]
#define C_GPIO_5    5                       ///< C_GPIO[5]
#define C_GPIO_6    6                       ///< C_GPIO[6]
#define C_GPIO_7    7                       ///< C_GPIO[7]
#define C_GPIO_8    8                       ///< C_GPIO[8]
#define C_GPIO_9    9                       ///< C_GPIO[9]
#define C_GPIO_10   10                      ///< C_GPIO[10]
#define C_GPIO_11   11                      ///< C_GPIO[11]
#define C_GPIO_12   12                      ///< C_GPIO[12]
#define C_GPIO_13   13                      ///< C_GPIO[13]
#define C_GPIO_14   14                      ///< C_GPIO[14]
#define C_GPIO_15   15                      ///< C_GPIO[15]
#define C_GPIO_16   16                      ///< C_GPIO[16]
#define C_GPIO_17   17                      ///< C_GPIO[17]
#define C_GPIO_18   18                      ///< C_GPIO[18]
#define C_GPIO_19   19                      ///< C_GPIO[19]
#define C_GPIO_20   20                      ///< C_GPIO[20]
#define C_GPIO_21   21                      ///< C_GPIO[21]
#define C_GPIO_22   22                      ///< C_GPIO[22]
#define C_GPIO_23   23                      ///< C_GPIO[23]
#define C_GPIO_24   24                      ///< C_GPIO[24]


#define H_GPIO_SHIFT_BASE 32
#define HSI_GPIO_0    (0 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[0]
#define HSI_GPIO_1    (1 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[1]
#define HSI_GPIO_2    (2 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[2]
#define HSI_GPIO_3    (3 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[3]
#define HSI_GPIO_4    (4 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[4]
#define HSI_GPIO_5    (5 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[5]
#define HSI_GPIO_6    (6 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[6]
#define HSI_GPIO_7    (7 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[7]
#define HSI_GPIO_8    (8 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[8]
#define HSI_GPIO_9    (9 +H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[9]
#define HSI_GPIO_10   (10+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[10]
#define HSI_GPIO_11   (11+H_GPIO_SHIFT_BASE)  ///< HSI_GPIO[11]

#define S_GPIO_SHIFT_BASE 64
#define S_GPIO_0    (0 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[0]
#define S_GPIO_1    (1 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[1]
#define S_GPIO_2    (2 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[2]
#define S_GPIO_3    (3 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[3]
#define S_GPIO_4    (4 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[4]
#define S_GPIO_5    (5 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[5]
#define S_GPIO_6    (6 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[6]
#define S_GPIO_7    (7 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[7]
#define S_GPIO_8    (8 +S_GPIO_SHIFT_BASE)  ///< S_GPIO[8]


#define P_GPIO_SHIFT_BASE 96
#define P_GPIO_0    (0 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[0]
#define P_GPIO_1    (1 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[1]
#define P_GPIO_2    (2 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[2]
#define P_GPIO_3    (3 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[3]
#define P_GPIO_4    (4 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[4]
#define P_GPIO_5    (5 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[5]
#define P_GPIO_6    (6 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[6]
#define P_GPIO_7    (7 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[7]
#define P_GPIO_8    (8 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[8]
#define P_GPIO_9    (9 +P_GPIO_SHIFT_BASE)  ///< P_GPIO[9]
#define P_GPIO_10   (10+P_GPIO_SHIFT_BASE)  ///< P_GPIO[10]
#define P_GPIO_11   (11+P_GPIO_SHIFT_BASE)  ///< P_GPIO[11]
#define P_GPIO_12   (12+P_GPIO_SHIFT_BASE)  ///< P_GPIO[12]
#define P_GPIO_13   (13+P_GPIO_SHIFT_BASE)  ///< P_GPIO[13]
#define P_GPIO_14   (14+P_GPIO_SHIFT_BASE)  ///< P_GPIO[14]
#define P_GPIO_15   (15+P_GPIO_SHIFT_BASE)  ///< P_GPIO[15]
#define P_GPIO_16   (16+P_GPIO_SHIFT_BASE)  ///< P_GPIO[16]
#define P_GPIO_17   (17+P_GPIO_SHIFT_BASE)  ///< P_GPIO[17]
#define P_GPIO_18   (18+P_GPIO_SHIFT_BASE)  ///< P_GPIO[18]
#define P_GPIO_19   (19+P_GPIO_SHIFT_BASE)  ///< P_GPIO[19]
#define P_GPIO_20   (20+P_GPIO_SHIFT_BASE)  ///< P_GPIO[20]
#define P_GPIO_21   (21+P_GPIO_SHIFT_BASE)  ///< P_GPIO[21]
#define P_GPIO_22   (22+P_GPIO_SHIFT_BASE)  ///< P_GPIO[22]
#define P_GPIO_23   (23+P_GPIO_SHIFT_BASE)  ///< P_GPIO[23]
#define P_GPIO_24   (24+P_GPIO_SHIFT_BASE)  ///< P_GPIO[24]
#define P_GPIO_25   (25+P_GPIO_SHIFT_BASE)  ///< P_GPIO[25]

#define DSI_GPIO_SHIFT_BASE 128
#define DSI_GPIO_0    (0 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[0]
#define DSI_GPIO_1    (1 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[1]
#define DSI_GPIO_2    (2 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[2]
#define DSI_GPIO_3    (3 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[3]
#define DSI_GPIO_4    (4 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[4]
#define DSI_GPIO_5    (5 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[5]
#define DSI_GPIO_6    (6 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[6]
#define DSI_GPIO_7    (7 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[7]
#define DSI_GPIO_8    (8 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[8]
#define DSI_GPIO_9    (9 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[9]
#define DSI_GPIO_10   (10 +DSI_GPIO_SHIFT_BASE)  ///< DSI_GPIO[10]

#define LCD_GPIO_SHIFT_BASE 160
#define L_GPIO_0    (0 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[0]
#define L_GPIO_1    (1 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[1]
#define L_GPIO_2    (2 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[2]
#define L_GPIO_3    (3 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[3]
#define L_GPIO_4    (4 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[4]
#define L_GPIO_5    (5 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[5]
#define L_GPIO_6    (6 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[6]
#define L_GPIO_7    (7 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[7]
#define L_GPIO_8    (8 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[8]
#define L_GPIO_9    (9 +LCD_GPIO_SHIFT_BASE)  ///< L_GPIO[9]

#define D_GPIO_SHIFT_BASE 192
#define D_GPIO_0    (0 +D_GPIO_SHIFT_BASE)  ///< DGPIO[0]
#define D_GPIO_1    (1 +D_GPIO_SHIFT_BASE)  ///< DGPIO[1]
#define D_GPIO_2    (2 +D_GPIO_SHIFT_BASE)  ///< DGPIO[2]
#define D_GPIO_3    (3 +D_GPIO_SHIFT_BASE)  ///< DGPIO[3]
#define D_GPIO_4    (4 +D_GPIO_SHIFT_BASE)  ///< DGPIO[4]
#define D_GPIO_5    (5 +D_GPIO_SHIFT_BASE)  ///< DGPIO[5]
#define D_GPIO_6    (6 +D_GPIO_SHIFT_BASE)  ///< DGPIO[6]
#define D_GPIO_7    (7 +D_GPIO_SHIFT_BASE)  ///< DGPIO[7]

#define A_GPIO_SHIFT_BASE 224
#define A_GPIO_0    (0 +A_GPIO_SHIFT_BASE)  ///< A_GPIO[0]
#define A_GPIO_1    (1 +A_GPIO_SHIFT_BASE)  ///< A_GPIO[1]
#define A_GPIO_2    (2 +A_GPIO_SHIFT_BASE)  ///< A_GPIO[2]
#define A_GPIO_3    (3 +A_GPIO_SHIFT_BASE)  ///< A_GPIO[3]

//@}
#if 0
/*For temporary usage as passing build codes*/
#define C_GPIO_23   23
#define C_GPIO_24   24
#define C_GPIO_25   25
#define C_GPIO_26   26
#define C_GPIO_27   27
#define C_GPIO_28   28
#define C_GPIO_29   29
#define C_GPIO_30   30
#define C_GPIO_31   31
#define C_GPIO_32   32
#define C_GPIO_33   33
#define L_GPIO_30   (30+L_GPIO_SHIFT_BASE)
#define L_GPIO_31   (31+L_GPIO_SHIFT_BASE)
#define L_GPIO_32   (32+L_GPIO_SHIFT_BASE)
#define GPIO_INT_USBPLUGIN (GPIO_INT_39)
#endif
#define L_GPIO_22	(22+LCD_GPIO_SHIFT_BASE)



/**
    @name   GPIO Interrupt ID

    GPIO interrupt ID definition

    GPIO interrupt ID for interrupt related APIs.
*/
//@{
#define GPIO_INT_00                 0       ///< GPIO INT[0]:   C_GPIO[0]
#define GPIO_INT_01                 1       ///< GPIO INT[1]:   C_GPIO[1]
#define GPIO_INT_02                 2       ///< GPIO INT[2]:   C_GPIO[2]
#define GPIO_INT_03                 3       ///< GPIO INT[3]:   C_GPIO[3]
#define GPIO_INT_04                 4       ///< GPIO INT[4]:   C_GPIO[4]
#define GPIO_INT_05                 5       ///< GPIO INT[5]:   C_GPIO[5]
#define GPIO_INT_06                 6       ///< GPIO INT[6]:   C_GPIO[6]
#define GPIO_INT_07                 7       ///< GPIO INT[7]:   C_GPIO[7]
#define GPIO_INT_08                 8       ///< GPIO INT[8]:   C_GPIO[8]
#define GPIO_INT_09                 9       ///< GPIO INT[9]:   C_GPIO[9]
#define GPIO_INT_10                 10      ///< GPIO INT[10]:  C_GPIO[10]
#define GPIO_INT_11                 11      ///< GPIO INT[11]:  C_GPIO[11]
#define GPIO_INT_12                 12      ///< GPIO INT[12]:  C_GPIO[12]
#define GPIO_INT_13                 13      ///< GPIO INT[13]:  C_GPIO[13]
#define GPIO_INT_14                 14      ///< GPIO INT[14]:  C_GPIO[14]
#define GPIO_INT_15                 15      ///< GPIO INT[15]:  C_GPIO[15]
#define GPIO_INT_16                 16      ///< GPIO INT[16]:  C_GPIO[16]
#define GPIO_INT_17                 17      ///< GPIO INT[17]:  C_GPIO[17]
#define GPIO_INT_18                 18      ///< GPIO INT[18]:  C_GPIO[18]
#define GPIO_INT_19                 19      ///< GPIO INT[19]:  C_GPIO[19]
#define GPIO_INT_20                 20      ///< GPIO INT[20]:  C_GPIO[20]
#define GPIO_INT_21                 21      ///< GPIO INT[21]:  C_GPIO[21]
#define GPIO_INT_22                 22      ///< GPIO INT[22]:  C_GPIO[22]
#define GPIO_INT_23                 23      ///< GPIO INT[23]:  C_GPIO[23]
#define GPIO_INT_24                 24      ///< GPIO INT[24]:  C_GPIO[24]
#define GPIO_INT_25                 25      ///< GPIO INT[25]:  HSI_GPIO[0]
#define GPIO_INT_26                 26      ///< GPIO INT[26]:  HSI_GPIO[1]
#define GPIO_INT_27                 27      ///< GPIO INT[27]:  HSI_GPIO[2]
#define GPIO_INT_28                 28      ///< GPIO INT[28]:  HSI_GPIO[3]
#define GPIO_INT_29                 29      ///< GPIO INT[29]:  HSI_GPIO[4]
#define GPIO_INT_30                 30      ///< GPIO INT[30]:  HSI_GPIO[5]
#define GPIO_INT_31                 31      ///< GPIO INT[31]:  HSI_GPIO[6]
#define GPIO_INT_32                 32      ///< GPIO INT[0]:   HSI_GPIO[7]
#define GPIO_INT_33                 33      ///< GPIO INT[1]:   HSI_GPIO[8]
#define GPIO_INT_34                 34      ///< GPIO INT[2]:   HSI_GPIO[9]
#define GPIO_INT_35                 35      ///< GPIO INT[3]:   HSI_GPIO[10]
#define GPIO_INT_36                 36      ///< GPIO INT[4]:   HSI_GPIO[11]
#define GPIO_INT_37                 37      ///< GPIO INT[5]:   S_GPIO[0]
#define GPIO_INT_38                 38      ///< GPIO INT[6]:   S_GPIO[1]
#define GPIO_INT_39                 39      ///< GPIO INT[7]:   S_GPIO[2]
#define GPIO_INT_40                 40      ///< GPIO INT[8]:   S_GPIO[3]
#define GPIO_INT_41                 41      ///< GPIO INT[9]:   S_GPIO[4]
#define GPIO_INT_42                 42      ///< GPIO INT[10]:  S_GPIO[5]
#define GPIO_INT_43                 43      ///< GPIO INT[11]:  S_GPIO[6]
#define GPIO_INT_44                 44      ///< GPIO INT[12]:  S_GPIO[7]
#define GPIO_INT_45                 45      ///< GPIO INT[13]:  S_GPIO[8]
#define GPIO_INT_46                 46      ///< GPIO INT[14]:  P_GPIO[0]
#define GPIO_INT_47                 47      ///< GPIO INT[15]:  P_GPIO[1]
#define GPIO_INT_48                 48      ///< GPIO INT[16]:  P_GPIO[2]
#define GPIO_INT_49                 49      ///< GPIO INT[17]:  P_GPIO[3]
#define GPIO_INT_50                 50      ///< GPIO INT[18]:  P_GPIO[4]
#define GPIO_INT_51                 51      ///< GPIO INT[19]:  P_GPIO[5]
#define GPIO_INT_52                 52      ///< GPIO INT[20]:  P_GPIO[6]
#define GPIO_INT_53                 53      ///< GPIO INT[21]:  P_GPIO[7]
#define GPIO_INT_54                 54      ///< GPIO INT[22]:  P_GPIO[8]
#define GPIO_INT_55                 55      ///< GPIO INT[23]:  P_GPIO[9]
#define GPIO_INT_56                 56      ///< GPIO INT[24]:  P_GPIO[10]
#define GPIO_INT_57                 57      ///< GPIO INT[25]:  P_GPIO[11]
#define GPIO_INT_58                 58      ///< GPIO INT[26]:  P_GPIO[12]
#define GPIO_INT_59                 59      ///< GPIO INT[27]:  P_GPIO[13]
#define GPIO_INT_60                 60      ///< GPIO INT[28]:  P_GPIO[14]
#define GPIO_INT_61                 61      ///< GPIO INT[29]:  P_GPIO[15]
#define GPIO_INT_62                 62      ///< GPIO INT[30]:  P_GPIO[16]
#define GPIO_INT_63                 63      ///< GPIO INT[31]:  P_GPIO[17]
#define GPIO_INT_64                 64      ///< GPIO INT[0]:   P_GPIO[18]
#define GPIO_INT_65                 65      ///< GPIO INT[1]:   P_GPIO[19]
#define GPIO_INT_66                 66      ///< GPIO INT[2]:   P_GPIO[20]
#define GPIO_INT_67                 67      ///< GPIO INT[3]:   P_GPIO[21]
#define GPIO_INT_68                 68      ///< GPIO INT[4]:   P_GPIO[22]
#define GPIO_INT_69                 69      ///< GPIO INT[5]:   P_GPIO[23]
#define GPIO_INT_70                 70      ///< GPIO INT[6]:   P_GPIO[24]
#define GPIO_INT_71                 71      ///< GPIO INT[7]:   dummy
#define GPIO_INT_72                 72      ///< GPIO INT[8]:   DSI_GPIO[0]
#define GPIO_INT_73                 73      ///< GPIO INT[9]:   DSI_GPIO[1]
#define GPIO_INT_74                 74      ///< GPIO INT[10]:  DSI_GPIO[2]
#define GPIO_INT_75                 75      ///< GPIO INT[11]:  DSI_GPIO[3]
#define GPIO_INT_76                 76      ///< GPIO INT[12]:  DSI_GPIO[4]
#define GPIO_INT_77                 77      ///< GPIO INT[13]:  DSI_GPIO[5]
#define GPIO_INT_78                 78      ///< GPIO INT[14]:  DSI_GPIO[6]
#define GPIO_INT_79                 79      ///< GPIO INT[15]:  DSI_GPIO[7]
#define GPIO_INT_80                 80      ///< GPIO INT[16]:  DSI_GPIO[8]
#define GPIO_INT_81                 81      ///< GPIO INT[17]:  DSI_GPIO[9]
#define GPIO_INT_82                 82      ///< GPIO INT[18]:  DSI_GPIO[10]
#define GPIO_INT_83                 83      ///< GPIO INT[19]:  L_GPIO[0]
#define GPIO_INT_84                 84      ///< GPIO INT[20]:  L_GPIO[1]
#define GPIO_INT_85                 85      ///< GPIO INT[21]:  L_GPIO[2]
#define GPIO_INT_86                 86      ///< GPIO INT[22]:  L_GPIO[3]
#define GPIO_INT_87                 87      ///< GPIO INT[23]:  L_GPIO[4]
#define GPIO_INT_88                 88      ///< GPIO INT[24]:  L_GPIO[5]
#define GPIO_INT_89                 89      ///< GPIO INT[25]:  L_GPIO[6]
#define GPIO_INT_90                 90      ///< GPIO INT[26]:  L_GPIO[7]
#define GPIO_INT_91                 91      ///< GPIO INT[27]:  L_GPIO[8]
#define GPIO_INT_92                 92      ///< GPIO INT[28]:  L_GPIO[9]
#define GPIO_INT_93                 93      ///< GPIO INT[29]:  D_GPIO[0]
#define GPIO_INT_94                 94      ///< GPIO INT[30]:  D_GPIO[1]
#define GPIO_INT_95                 95      ///< GPIO INT[31]:  D_GPIO[2]
#define GPIO_INT_96                 96      ///< GPIO INT[0]:   D_GPIO[3]
#define GPIO_INT_97                 97      ///< GPIO INT[1]:   D_GPIO[4]
#define GPIO_INT_98                 98      ///< GPIO INT[2]:   D_GPIO[5]
#define GPIO_INT_99                 99      ///< GPIO INT[3]:   D_GPIO[6]
#define GPIO_INT_100               100      ///< GPIO INT[4]:   D_GPIO[7]
#define GPIO_INT_101               101      ///< GPIO INT[5]:   A_GPIO[0]
#define GPIO_INT_102               102      ///< GPIO INT[6]:   A_GPIO[1]
#define GPIO_INT_103               103      ///< GPIO INT[7]:   A_GPIO[2]
#define GPIO_INT_104               104      ///< GPIO INT[8]:   A_GPIO[3]
#define GPIO_INT_105               105      ///< internal gpio 0
#define GPIO_INT_106               106      ///< internal gpio 1
#define GPIO_INT_107               107      ///< internal gpio 2
#define GPIO_INT_108               108      ///< internal gpio 3
#define GPIO_INT_109               109      ///< internal gpio 4
#define GPIO_INT_110               110      ///< internal gpio 5
#define GPIO_INT_USBWAKEUP         111      ///< USB wakeup
#define GPIO_INT_USBPLUGIN         (GPIO_INT_USBWAKEUP)
#define GPIO_INT_SWTRIG            GPIO_INT_110
//@}

// In order to backward comaptible, DGPIO is used as " GPIO_IS_DGPIO | D_GPIO_* " or "D_GPIO_*"
#define GPIO_IS_DGPIO               (128)


//
// The general api for the GPIO device driver
//
extern void     gpio_setDir(UINT32 pin, UINT32 dir);
extern UINT32   gpio_getDir(UINT32 pin);
extern void     gpio_setPin(UINT32 pin);
extern void     gpio_clearPin(UINT32 pin);
extern UINT32   gpio_getPin(UINT32 pin);
extern void     gpio_pullSet(UINT32 pin);
extern void     gpio_pullClear(UINT32 pin);
extern UINT32   gpio_readData(UINT32 dataidx);
extern void     gpio_writeData(UINT32 dataidx, UINT32 value);
//extern UINT32   gpio_readDir(UINT32 dataidx);
//extern void     gpio_writeDir(UINT32 dataidx, UINT32 value);
extern UINT32   dgpio_readData(void);
extern void     dgpio_writeData(UINT32 value);
extern UINT32 	top_get_bs(void);
//extern UINT32   dgpio_readDir(void);
//extern void     dgpio_writeDir(UINT32 value);
//@}
#endif

