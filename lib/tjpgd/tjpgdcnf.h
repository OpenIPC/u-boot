/*----------------------------------------------*/
/* TJpgDec System Configurations R0.03          */
/* (C)ChaN, 2021; values for U-Boot's bootlogo  */
/*----------------------------------------------*/

#define	JD_SZBUF		1024
/* Size of stream input buffer */

#define JD_FORMAT		0
/* Output pixel format 0:RGB888 (3 BYTE/pix), 1:RGB565 (1 WORD/pix), 2:Grayscale (1 BYTE/pix) */

#define	JD_USE_SCALE	0
/* Switches output descaling feature: 0:Disable, 1:Enable */

#define JD_TBLCLIP		1
/* Use table conversion for saturation arithmetic. A bit faster, but increases 1 KB of code size. */

#define JD_FASTDECODE	1
/* Optimization level: 0: basic, 1: + 32-bit barrel shifter, 2: + table conversion for huffman decoding */
