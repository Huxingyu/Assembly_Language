;实验一 汇编程序、链接程序、编辑程序及调试程序的基本使用方法
;一、实验目的
;1．熟悉在PC机上建立、汇编、连接、调试和运行汇编语言程序的过程。
;2．掌握字符串的复制，并会用各种方法实现。
;3．掌握字符串显示的DOS功能调用。
;二、实验内容及要求
;1．将以符号地址为ARRAY1开始的字符串“请自己定义”复制到ARRAY2开始的存储单元中并显示在屏幕上。
;2. 使用三种方法传送：
;(1) 用MOV指令传送
;(2) 用基本串传送指令
;(3) 重复串传送指令



MY_DATA 	SEGMENT	PARA 'DATA';数据段                   
    DISP1  DB 'How are you?',0aH,0DH,'$';
    ARRAY1 DB 'DI',41H,'NZI09','$';
    ARRAY2 DB 20 dup(0)                
MY_DATA 	ENDs

MY_CODE 	SEGMENT PARA 'CODE' ;代码段

MY_PROC		PROC	FAR
	
	ASSUME 	CS:MY_CODE, DS:MY_DATA
START:	MOV  AX,MY_DATA
	MOV  DS,AX
	MOV  ES,AX

        LEA  DX,DISP1;显示提示字符串
        MOV  AH,09H
	INT  21H

       ;;开始(1) 用MOV指令传送         
       ;LEA  SI,ARRAY1;
       ;LEA  DI,ARRAY2;
       ;MOV  CX,09H
LOOP1: ;MOV  BH, [SI]
       ;MOV  [DI], BH
       ;INC  SI
       ;INC  DI
       ;LOOP LOOP1

       ;;开始(2) 用基本串传送指令
       LEA  SI,ARRAY1;
       LEA  DI,ARRAY2;
       MOV  CX,09H
       CLD
LOOP2: MOVSB
       LOOP LOOP2

       ;;开始(3) 重复串传送指令
       ;LEA  SI,ARRAY1;
       ;LEA  DI,ARRAY2;
       ;MOV  CX,09H
       ;CLD
       ;REP MOVSB

       ;将ARRY2中的字符串显示出来，便于判断是否传送正确。
        LEA DX,ARRAY2;显示信息
        MOV AH,09H
        INT 21H

EXIT:	MOV  AX,4C00H
	INT  21H

MY_PROC	  ENDp		
		

MY_CODE   ENDS

	  END    START	