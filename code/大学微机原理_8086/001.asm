;ʵ��һ ���������ӳ��򡢱༭���򼰵��Գ���Ļ���ʹ�÷���
;һ��ʵ��Ŀ��
;1����Ϥ��PC���Ͻ�������ࡢ���ӡ����Ժ����л�����Գ���Ĺ��̡�
;2�������ַ����ĸ��ƣ������ø��ַ���ʵ�֡�
;3�������ַ�����ʾ��DOS���ܵ��á�
;����ʵ�����ݼ�Ҫ��
;1�����Է��ŵ�ַΪARRAY1��ʼ���ַ��������Լ����塱���Ƶ�ARRAY2��ʼ�Ĵ洢��Ԫ�в���ʾ����Ļ�ϡ�
;2. ʹ�����ַ������ͣ�
;(1) ��MOVָ���
;(2) �û���������ָ��
;(3) �ظ�������ָ��



MY_DATA 	SEGMENT	PARA 'DATA';���ݶ�                   
    DISP1  DB 'How are you?',0aH,0DH,'$';
    ARRAY1 DB 'DI',41H,'NZI09','$';
    ARRAY2 DB 20 dup(0)                
MY_DATA 	ENDs

MY_CODE 	SEGMENT PARA 'CODE' ;�����

MY_PROC		PROC	FAR
	
	ASSUME 	CS:MY_CODE, DS:MY_DATA
START:	MOV  AX,MY_DATA
	MOV  DS,AX
	MOV  ES,AX

        LEA  DX,DISP1;��ʾ��ʾ�ַ���
        MOV  AH,09H
	INT  21H

       ;;��ʼ(1) ��MOVָ���         
       ;LEA  SI,ARRAY1;
       ;LEA  DI,ARRAY2;
       ;MOV  CX,09H
LOOP1: ;MOV  BH, [SI]
       ;MOV  [DI], BH
       ;INC  SI
       ;INC  DI
       ;LOOP LOOP1

       ;;��ʼ(2) �û���������ָ��
       LEA  SI,ARRAY1;
       LEA  DI,ARRAY2;
       MOV  CX,09H
       CLD
LOOP2: MOVSB
       LOOP LOOP2

       ;;��ʼ(3) �ظ�������ָ��
       ;LEA  SI,ARRAY1;
       ;LEA  DI,ARRAY2;
       ;MOV  CX,09H
       ;CLD
       ;REP MOVSB

       ;��ARRY2�е��ַ�����ʾ�����������ж��Ƿ�����ȷ��
        LEA DX,ARRAY2;��ʾ��Ϣ
        MOV AH,09H
        INT 21H

EXIT:	MOV  AX,4C00H
	INT  21H

MY_PROC	  ENDp		
		

MY_CODE   ENDS

	  END    START	