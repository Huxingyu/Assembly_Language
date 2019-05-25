#include "reg52.h"
#include "main.h"
#include "mfrc522.h"
#include<intrins.h>         //函数处理头文件	
#include<string.h>          //字符串处理头文件  
#include<stdio.h>           // 输入输出函数头文件

/********************************************************************************************/
//数值类别
typedef signed   char      int8;                 // 有符号8位整型变量
typedef unsigned char      uint8;                // 无符号8位整型变量
typedef signed   int       int16;                // 有符号16位整型变量
typedef unsigned int       uint16;               // 无符号16位整型变量
/********************************************************************************************/
// 定义特殊寄存器                       
sfr wdt_contr=0xe1;								 //定义看门狗地址 
sfr isp_data =0xe2;								 //定义ISP数据寄存器
sfr isp_addrh=0xe3;								 //定义ISP地址高8位
sfr isp_addrl=0xe4;								 //定义ISP地址低8位
sfr isp_cmd  =0xe5;								 //定义ISP命令模式寄存器
sfr isp_trig =0xe6;								 //定义ISP命令触发寄存器
sfr isp_contr=0xe7;								 //定义ISP控制模式寄存器


unsigned char code data1[16] = {0xC8,0x00,0x00,0x00,0x37,0xFF,0xFF,0xFF,0xC8,0x00,0x00,0x00,0x01,0xFE,0x01,0xFE};//200
//M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
//4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反 
unsigned char data2[4]  = {0x00,0x00,0x00,0x01};//存值减值金额
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //密码
unsigned char TYPE_DATA[2];//类型
unsigned char ID_DATA[4];  //ID地址 
unsigned char value_DATA[4];//卡里面的值					                            					                      
unsigned char g_ucTempbuf[20];//
unsigned char code dis0[] = {"Total:"};
unsigned char code dis1[] = {" "}; 
unsigned char code dis2[] = {"Card ID:"};
unsigned char code dis3[] = {"Met:"};        
unsigned char code dis4[] = {" Card:"};  
   
unsigned char code dis6[] = {"0123456789ABCDEF"};  
unsigned int tt=0,keyfalg,falg_pro;  
unsigned char mc;
uint16 tot=0,mon=0,card=0;
uint8  a=0,b=0,c=0,d=0,ds0,ds1;
/********************************************************************************************/

//EEPROM驱动开始
/*********************************************************************************
* 函数名称：eepromEraseSector (uint16 address)  
* 功    能：EEPROM擦除程序
* 入口参数：address
* 出口参数：无
**********************************************************************************/
//擦除片内EEPROM的一个扇区   
//擦除只能以扇区为最小单位进行，没法只擦除一个字节   
//一个扇区是512个字节   
//本函数参数里面的地址落在哪个扇区，则该扇区内数据都将被擦除   
//例如：STC89C51RC片内EEPROM第一扇区开始地址为0x2000，结束地址为0x21ff   
//如果调用 eepromEraseSector(0x2001)，则第一扇区内数据都将被擦除   
//擦除成功后，该扇区内各字节都将变为0xff   
void eepromEraseSector (uint16 address)  
{  
	uint8 i;  
	isp_addrl=address;  
	isp_addrh=address>>8;  
	isp_contr=0x01;   
	isp_contr=isp_contr|0x81; // 0x80 if SYSCLK<40MHz, 0x81 if SYSCLK<20MHz, 0x82 if SYSCLK<10MHz, 0x83 if SYSCLK<5MHz   
	isp_cmd=0x03;   //送擦除扇区命令
	isp_trig=0x46;  
	isp_trig=0xb9;  //触发寄存器
	for(i=0;i<3;i++);  
		isp_contr=0x00;
		isp_cmd  =0x00; 
		isp_trig =0x00; 
		isp_addrl=0x0;  
		isp_addrh=0x0;  	
}  
/**********************************************************************************
* 函数名称：eepromWrite(uint16 address, uint8 write_data)    
* 功    能：EEPROM写程序
* 入口参数：address，write_data
* 出口参数：无
***********************************************************************************/
//对STC片内EEPROM的指定地址写入数据（即，字节编程）。   
//注意：字节编程是指将eeprom的1写成1或0，将0写成0，而无法将0写成1   
//所以，在写入数据前，一定要用扇区擦除将所有字节变为0xff 
void eepromWrite(uint16 address, uint8 write_data)  
{  
	uint8 i;  
	isp_data=write_data;  
	isp_addrl=address;  
	isp_addrh=address>>8;  
	isp_contr=0x01;   
	isp_contr=isp_contr|0x81; // 0x80 if SYSCLK<40MHz, 0x81 if SYSCLK<20MHz, 0x82 if SYSCLK<10MHz, 0x83 if SYSCLK<5MHz   
	isp_cmd=0x02;  //送写数据命令
	isp_trig=0x46;  
	isp_trig=0xb9;  //触发寄存器
	for(i=0;i<3;i++);  
		isp_contr=0x00;
		isp_cmd  =0x00; 
		isp_trig =0x00; 
		isp_addrl=0x0;  
		isp_addrh=0x0; 
}  
/***********************************************************************************
* 函数名称：eepromRead(uint16 address)    
* 功    能：EEPROM读程序
* 入口参数：address
* 出口参数：z
************************************************************************************/
//读取STC单片机内部EEPROM的一个字节   
//主要不同的STC单片机EEPROM起始地址不同   
//例如：STC89c52RC的片内EEPROM起始地址为0x2000
uint8 eepromRead(uint16 address)  
{  
	uint8 i,z;  
	isp_addrl=address;  
	isp_addrh=address>>8;  
	isp_contr=0x01;  
	isp_contr=isp_contr|0x81; // 0x80 if SYSCLK<40MHz, 0x81 if SYSCLK<20MHz, 0x82 if SYSCLK<10MHz, 0x83 if SYSCLK<5MHz   
	isp_cmd=0x01;  //送读数据命令
	isp_trig=0x46;  
	isp_trig=0xb9;  //触发寄存器
	for(i=0;i<3;i++);  
		isp_contr=0x00;
		isp_cmd  =0x00; 
		isp_trig =0x00; 
		isp_addrl=0x0;  
		isp_addrh=0x0;   
		z=isp_data;  
		return(z);  
	}  
void wreeprom(void){
		a=tot/256;
		b=tot%256;
		c=mon/256;
		d=mon%256;
		eepromEraseSector (0x2000);
		eepromWrite(0x2000, a);
		eepromWrite(0x2001, b);
		eepromWrite(0x2002, c);
		eepromWrite(0x2003, d);
		eepromWrite(0x2020, 0x00);
}
void eeprominit(void)
{
	unsigned char test;
	test=eepromRead(0x2020);
	while(test==0xff);
}

//中断程序
/**********************************************************************************
* 函数名称：time0_1_int()
* 功    能：定时中断0/1初始化
**********************************************************************************/
void time0_1_int(void) {
	TMOD=0x11;//time0,1方式1,16位定时器
	
	TH0=0xdc;//装载初始值（10ms）
	TL0=0x00;
	ET0=1;//开T0中断
	TR0=1;//启动定时中断T0
	
	EA=1;//开中断
	}

/********************************************************************************
* 函数名称：time0_fun()interrupt1
* 功    能：定时中断0服务子程序
********************************************************************************/
void time0_fun()interrupt 1 {
	TH0=0xdc;//重装载初始值10ms
	TL0=0x00;
	ds0++;
	ds1++;
	if(ds0>20){ds0=21;}
}
/*********************************************************************************
* 函数名称：int0_1_int()
* 功    能：外部中断0/1初始化
**********************************************************************************/
void int0_1_int(void) {
//	IT0=1;//下降沿触发
//	EX0=1;//TF0中断开
	IT1=1;//下降沿触发
	EX1=1;//TF1中断开
	EA=1;//开中断
	}
	
/*********************************************************************************
* 函数名称：int1_fun()interrupt2
* 功    能：外部中断1服务子程序
*********************************************************************************/
void int1_fun()interrupt 2 {
	mc++;
	if (mc>=1){
		mc=0;
		if (mon>0){mon--;tot++;}

		wreeprom();
		}		
	}
/**********************************************************************************
* 函数名称：init()
* 功    能：串行中断初始化
* 入口参数：无
* 出口参数：无
**********************************************************************************/

void init_con(void) 
{
	TMOD=0x21;//定时器T1方式2
	TH1=0xfd;//波特率9600
	TL1=0xfd;//波特率9600
	SCON=0x50;//串口方式1，10位可变波特率，允许接收
	PCON=0x00;
	TR1=1;//启动T1
	ES=1;//串口1开中断
	EA=1;//开总中断
}

/*********************************************************************************
* 函数名称：serial_serve() interrupt 4
* 功    能：串行通信中断服务子程序
* 入口参数：无
* 出口参数：无
**********************************************************************************/

void serial_serve(void) interrupt 4
{
	RI=0;
		}
	

void Sendchar(uint8 c)	     
{
	SBUF=c;                                     
	while(TI==0);                                               
	TI=0;
}	

/////////////////////////////////////////////////////////////////////
//系统初始化
/////////////////////////////////////////////////////////////////////
void InitializeSystem(void)
{     
     P0 = 0xFF;P1 = 0xFF;P2 = 0xFF;P3 = 0xFF;
	 fmq=0;
	 lcd_init();				// 初始化LCD			
	 Delay(1);
	 fmq=1;
}
void Delay(unsigned int time)
{
  unsigned int i,k  ;
  for(i=0;i<255;i++)
    for(k=0;k<time;k++)
     _nop_();	 
}
////////////////////////显示处理函数//////////////////////////////// 
bit lcd_bz()	   	         // 测试LCD忙碌状态
{						
	bit result;
	rs = 0;rw = 1;ep = 1;
	_nop_();_nop_();_nop_();_nop_();
	result = (bit)(P0 & 0x80);
	ep = 0;
	return result;	
}
void lcd_wcmd(unsigned char cmd)// 写入指令数据到LCD
{							
	while(lcd_bz());
	rs = 0;rw = 0;ep = 0;
	_nop_();_nop_();	
	P0 = cmd;
	_nop_();_nop_();_nop_();_nop_();
	ep = 1;
	_nop_();_nop_();_nop_();_nop_();
	ep = 0;		
}

void lcd_pos(unsigned char pos)
{							//设定显示位置
	lcd_wcmd(pos | 0x80);
}

void lcd_wdat(unsigned char dat)	
{							//写入字符显示数据到LCD
	while(lcd_bz());
	rs = 1;rw = 0;ep = 0;
	P0 = dat;
	_nop_();_nop_();_nop_();_nop_();
	ep = 1;
	_nop_();_nop_();_nop_();_nop_();
	ep = 0;	
}
void lcd_init(void)					 //LCD初始化设定
{							
	lcd_wcmd(0x38);Delay(1);	//
	lcd_wcmd(0x0c);Delay(1);	//
	lcd_wcmd(0x06);Delay(1);	//
	lcd_wcmd(0x01);Delay(1);	//清除LCD的显示内容
}
void LCD1206a(void)
{
	unsigned char i;
	TYPE_DATA[0]=0x04;
	lcd_pos(0x00);				// 设置显示位置为第一行的第0个字符
	i = 0;
	switch (TYPE_DATA[0])
	{
	case 0x04:
		lcd_wdat(dis0[0]);
		lcd_wdat(dis0[1]);
		lcd_wdat(dis0[2]);
		lcd_wdat(dis0[3]);
		lcd_wdat(dis0[4]);
		lcd_wdat(dis0[5]);
		break;
	case 0x02:
		lcd_wdat(dis1[0]);
		break;
	case 0x08:
		lcd_wdat(dis2[0]);
		break;
	case 0x44:
		lcd_wdat(dis3[0]);
		break;
	}
	lcd_wdat(dis6[tot/10000]);
	lcd_wdat(dis6[tot%10000/1000]);
	lcd_wdat(dis6[tot%10000%1000/100]);
	lcd_wdat(dis6[tot%10000%1000%100/10]);
	lcd_wdat(dis6[tot%10000%1000%100%10]);
	lcd_wdat(dis1[0]);
	lcd_wdat(dis1[0]);
	lcd_wdat(dis1[0]);
	lcd_wdat(dis1[0]);
	lcd_wdat(dis1[0]);
	lcd_pos(0x40);
	lcd_wdat(dis3[0]);
	lcd_wdat(dis3[1]);
	lcd_wdat(dis3[2]);
	lcd_wdat(dis3[3]);
	lcd_wdat(dis6[mon%10000%1000/100]);
	lcd_wdat(dis6[mon%10000%1000%100/10]);
	lcd_wdat(dis6[mon%10000%1000%100%10]);
	lcd_wdat(dis4[0]);
	lcd_wdat(dis4[1]);
	lcd_wdat(dis4[2]);
	lcd_wdat(dis4[3]);
	lcd_wdat(dis4[4]);
	lcd_wdat(dis4[5]);
	lcd_wdat(dis6[card%10000%1000/100]);
	lcd_wdat(dis6[card%10000%1000%100/10]);
	lcd_wdat(dis6[card%10000%1000%100%10]);
}  
void LCD1206b(void )
{
	unsigned char i;
	TYPE_DATA[0]=0x04;
	lcd_pos(0x00);				// 设置显示位置为第一行的第0个字符
	i = 0;
	switch (TYPE_DATA[0])
	{
	case 0x04:
		lcd_wdat(dis2[0]);
		lcd_wdat(dis2[1]);
		lcd_wdat(dis2[2]);
		lcd_wdat(dis2[3]);
		lcd_wdat(dis2[4]);
		lcd_wdat(dis2[5]);
		lcd_wdat(dis2[6]);
		lcd_wdat(dis2[7]);

		break;
		lcd_wdat(dis2[6]);
		break;
		lcd_wdat(dis2[7]);
		break;
	case 0x02:
		lcd_wdat(dis1[0]);
		break;
	case 0x08:
		lcd_wdat(dis2[0]);
		break;
	case 0x44:
		lcd_wdat(dis3[0]);
		break;
	}
	lcd_wdat(dis6[ID_DATA[0]%100/10]);
	lcd_wdat(dis6[ID_DATA[0]%10]);
	lcd_wdat(dis6[ID_DATA[1]%100/10]);
	lcd_wdat(dis6[ID_DATA[1]%10]);
	lcd_wdat(dis6[ID_DATA[2]%100/10]);
	lcd_wdat(dis6[ID_DATA[2]%10]);
	lcd_wdat(dis6[ID_DATA[3]%100/10]);
	lcd_wdat(dis6[ID_DATA[3]%10]);

	lcd_pos(0x40);
	lcd_wdat(dis3[0]);
	lcd_wdat(dis3[1]);
	lcd_wdat(dis3[2]);
	lcd_wdat(dis3[3]);
	lcd_wdat(dis6[mon%10000%1000/100]);
	lcd_wdat(dis6[mon%10000%1000%100/10]);
	lcd_wdat(dis6[mon%10000%1000%100%10]);
	lcd_wdat(dis4[0]);
	lcd_wdat(dis4[1]);
	lcd_wdat(dis4[2]);
	lcd_wdat(dis4[3]);
	lcd_wdat(dis4[4]);
	lcd_wdat(dis4[5]);
	lcd_wdat(dis6[card%10000%1000/100]);
	lcd_wdat(dis6[card%10000%1000%100/10]);
	lcd_wdat(dis6[card%10000%1000%100%10]);
} 

void pro()//蜂鸣器滴一声
{
	fmq=0;
	ds0=0;
}

void readk()//读卡
{	
unsigned char status;
 status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A 验证密码模式60A密码 61B密码，“1”块地址，DefaultKey初始密码，g_ucTempbuf ID号
	 if (status == MI_OK) //验证密码
	 {	 																																														  
    status = PcdRead(1, g_ucTempbuf);//读块地址“1”的数据，返回值存在	g_ucTempbuf
			  if (status == MI_OK)
			  { 
				value_DATA[0]=g_ucTempbuf[0];
				value_DATA[1]=g_ucTempbuf[1];
				value_DATA[2]=g_ucTempbuf[2];
				value_DATA[3]=g_ucTempbuf[3];	
				card=value_DATA[0]+value_DATA[1]*256;										     
			  }   
     }
}	
void writek()//写卡	存值或减值
{
    unsigned char status;
    status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A 验证密码模式60A密码 61B密码，“1”块地址，DefaultKey初始密码，g_ucTempbuf ID号
	  if (status == MI_OK) //验证密码
	  {	
					data2[1]=100/256;data2[0]=100%256;//扣款金额
					status = PcdValue(PICC_DECREMENT,1,data2);//扣款存值命令，钱包地址，金额 低位在前  PICC_INCREMENT 充值
					if (status == MI_OK)
					{
					pro();
					 
					mon=mon+100;                                     //充值金额
					wreeprom();
				    }       
					status = PcdBakValue(1, 2);              //备份钱包	  “1”源地址“2”目标地址
					if(status == MI_OK){};
			 
		}

}	
void init_k(void)//初始化卡
{
    unsigned char status;
		if(key2==0)                              //充值
		{
	    status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//PICC_AUTHENT1A 验证密码模式60A密码 61B密码，“1”块地址，DefaultKey初始密码，g_ucTempbuf ID号
		  if (status == MI_OK) //验证密码
		  {																																								  	 
				  status = PcdWrite(1, data1);//4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反
				  if (status == MI_OK)
					 { 
					 	 pro(); 
					 } 
			}
		}
}

void main( )
{    
     unsigned char status;
 	 time0_1_int();//定时中断初始化
	 int0_1_int();
	 init_con();
     InitializeSystem( );//初始化

     PcdReset();	    //复位RC522
     PcdAntennaOff(); //关闭天线
     PcdAntennaOn();  //开启天线

	a=eepromRead(0x2000);//从eeprom读
	b=eepromRead(0x2001);//从eeprom读
	c=eepromRead(0x2002);//从eeprom读
	d=eepromRead(0x2003);//wreeprom

	
	if (a==0xff&b==0xff&c==0xff&d==0xff) {
		a=0,b=0,c=0,d=0;
		eepromEraseSector (0x2000);//擦除eeprom
		eepromWrite(0x2000, a);//写入eeprom
		eepromWrite(0x2001, b);//写入eeprom
		eepromWrite(0x2002, c);//写入eeprom
		eepromWrite(0x2003, d);}//写入eeprom
	tot=a*256+b;
	mon=c*256+d;
	eeprominit();
	while ( 1 )
     { 

		if(key1==0){dcf=1;dcfzsd=1;fmq=0;kgbj=0;}else{fmq=1;kgbj=1;}//开盖检测到开启时关阀门、报警
		if(key4==0){tot=0;mon=0;wreeprom();}
        status = PcdRequest(PICC_REQALL, g_ucTempbuf); //返回卡片类型#define PICC_REQIDL 0x26 寻天线区内未进入休眠状态
		if (status != MI_OK){TYPE_DATA[0]=0;TYPE_DATA[1]=0;
							ID_DATA[0]=0;ID_DATA[1]=0;ID_DATA[2]=0;ID_DATA[3]=0;
							value_DATA[0]=0;	value_DATA[1]=0;	value_DATA[2]=0;	value_DATA[3]=0;
							falg_pro=0;
							card=0;}//读不到卡，显示清0；
        if (status == MI_OK){TYPE_DATA[0]=g_ucTempbuf[0];TYPE_DATA[1]=g_ucTempbuf[1];
							status = PcdAnticoll(g_ucTempbuf);       //防冲撞 ，返回卡片ID号 4字节
							if (status == MI_OK){status = PcdSelect(g_ucTempbuf);    //选定卡片  ,输入卡片ID号
										if (status == MI_OK){if(falg_pro==0){falg_pro=1;pro();}//声音提示标志
													ID_DATA[0]=g_ucTempbuf[0];
													ID_DATA[1]=g_ucTempbuf[1];
													ID_DATA[2]=g_ucTempbuf[2];
													ID_DATA[3]=g_ucTempbuf[3];
													                            //
													if (key2==0 ){init_k();pro();}//初始化卡
													
													if (key2!=0 & mon<900 & card>0){writek();}//写卡 
													readk(); //读卡
													PcdHalt();//命令卡片进入休眠状态	

													} 	  
												}
							}   
		if(key3==0){Delay(40);if (mon>0){mon--;tot++;}
		wreeprom();}//写入eeprom}
		if(mon>0&key1!=0){dcf=0;dcfzsd=0;}else{dcf=1;dcfzsd=1;}//有余额时开阀
		
		if(mon<10){qqbj=0;}else{qqbj=1;}//余额低于10时报警灯亮;
		if(mon<10&mon>0){fmq=0;}else{fmq=1;}

		if (falg_pro){LCD1206b();}else{LCD1206a();};//显示处理	
		if (ds1 > 250 ){ 
				ds1=0;

				Sendchar(84);//发送“T”
				Sendchar(111);//发送“o”
				Sendchar(116);//发送“t”
				Sendchar(97);//发送“a” 
				Sendchar(108);//发送“l”
				Sendchar(58);//发送“:”
				Sendchar((tot%100000/10000)+0x30);//发送第1位
				Sendchar((tot%10000/1000)+0x30);//发送第2位
				Sendchar((tot%1000/100)+0x30);//发送第3位
				Sendchar((tot%100/10)+0x30);//发送第4位
				Sendchar((tot%10)+0x30);//发送第5位
				Sendchar(32);//发送“ ”
				Sendchar(32);//发送“ ”
				
				Sendchar(82);//发送“R”
				Sendchar(77);//发送“M”
				Sendchar(66);//发送“B”
				Sendchar(58);//发送“:”
				Sendchar((mon%1000/100)+0x30);//发送第1位
				Sendchar((mon%100/10)+0x30);//发送第2位
				Sendchar((mon%10)+0x30);//发送第3位
				Sendchar(32);//发送“ ”
				Sendchar(10);//发送“换行”

					}
		if(ds0>20){fmq=1;}
	}       
}