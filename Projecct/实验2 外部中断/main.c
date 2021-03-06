/****************************************************************************
* 文 件 名: main.c
* 描    述: 以外部中断方式在按键KEY3控制LED1.LED2.LED3实现倒流水灯效果和LED1闪烁效果之间切换
****************************************************************************/
#include <ioCC2530.h>

typedef unsigned char uchar;
typedef unsigned int  uint;

#define LED1 P1_0       //定义P1.0口为LED1控制端
#define LED2 P1_1       //定义P1.1口为LED2控制端
#define LED3 P1_4       //定义P1.4口为LED3控制端

#define KEY3 P0_7       // P0.1口控制KEY3
//===修改
#define KEY2 P2_0       // P2.0口控制按键KEY2
unsigned char flag_Pause=0; // 跑马灯运行标志位，1为暂停，0为运行
//===修改

#define ON      1
#define OFF     0

uchar KeyValue=0;

/****************************************************************************
* 名    称: DelayMS()
* 功    能: 以毫秒为单位延时，系统时钟不配置时默认为16M(用示波器测量相当精确)
* 入口参数: msec 延时参数，值越大，延时越久
* 出口参数: 无
****************************************************************************/
void DelayMS(uint msec)
{ 
    uint i,j;
    static int DelayCallCount=0;
    
    for (i=0; i<msec; i++)
        for (j=0; j<535; j++);
        //===修改
        {
          asm("NOP"); // asm是内嵌汇编，nop执行一个指令周期
          asm("NOP");
          asm("NOP");
          while(flag_Pause); // 根据flag_Pause的值确定是否在此循环
        }
        //===修改
    DelayCallCount++;
}

/****************************************************************************
* 名    称: LedOnOrOff()
* 功    能: 点亮或熄灭所有LED灯    
* 入口参数: mode为1时LED灯亮  mode为0时LED灯灭
* 出口参数: 无
****************************************************************************/
void LedOnOrOff(uchar mode)
{
    LED1 = mode;
    LED2 = mode;
    LED3 = mode;
}

/****************************************************************************
* 名    称: InitLed()
* 功    能: 设置LED灯相应的IO口
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitLed(void)
{

  P1DIR |= 0x01;               //P1.0定义为输出口  
  P1DIR |= 0x02;               //P1.1定义为输出口 
  P1DIR |= 0x10;               //P1.4定义为输出口 
  LedOnOrOff(0);      //使所有LED灯默认为熄灭状态
}

/****************************************************************************
* 名    称: InitKey()
* 功    能: 设置KEY相应的IO口，采用中断方式 
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitKey()
{   
  P0SEL &= ~0x80;     //设置P0.7为普通IO口  
  P0DIR &= ~0x80;     //按键接在P0.7口上，设P0.7为输入模式 
  P0INP &= ~0x80;     //打开P0.7上拉电阻 
  
  P0IEN |= 0x80;  // P0.7 设置为中断方式 
  PICTL |= 0x01; // 下降沿触发  
  //IEN1 |= 0x20;   // 允许P0口中断:P0中断使能位为 IEN1 的bit5
  P0IE = 1; // 允许P0口中断;
  P0IFG = 0x00;   // 初始化中断标志位
  
  //===修改
  P2SEL &= ~0x01;     //设置P2.0为普通IO口  
  P2DIR &= ~0x01;     //按键接在P2.0口上，设P2.0为输入模式 
  P2INP &= ~0x01;     //打开P2.0上拉电阻
  
  P2IEN |= 0x01;  // P2.0 设置为中断方式
  PICTL |= 0x01; // 下降沿触发 
  IEN2 |= 0x02; // 允许 P2 口中断:P2中断使能位为 IEN2 的bit1
  P2IFG = 0x00;   // 初始化中断标志位
  //===修改
  
  EA = 1; 
}

/****************************************************************************
* 名    称: P0_ISR(void) 中断处理函数 
* 描    述: #pragma vector = 中断向量，紧接着是中断处理程序
****************************************************************************/
#pragma vector = P0INT_VECTOR    
__interrupt void P0_ISR(void) 
{ 
    if( P0IFG & 0x80 )          //按键P0.7中断
    {
      DelayMS(10);       //延时去抖     
       if(KEY3==0)       
       {
           KeyValue = !KeyValue;  //产生中断保存中断状态
       }  
    } 
    
    P0IFG &= ~ 0x80;             //清Pin0.1中断标志
    P0IF = 0;              //清端口0中断标志
} 

//===修改
#pragma vector = P2INT_VECTOR  //#pragma vector = 中断向量，紧接着是中断处理程序    
__interrupt void P2_ISR(void) 
{ 
    if( P2IFG & 0x01 )  //按键P2.0中断
    {
      if(flag_Pause == 0)
      {
          flag_Pause =1;
      }
      else
      {
        flag_Pause =0;
      }
      P2IFG &= ~ 0x01;             //清Pin2.0中断标志
    }
    P2IF = 0;              //清端口2中断标志
} 
//===修改

/****************************************************************************
* 程序入口函数
****************************************************************************/
void main(void)
{
     InitLed();             //设置LED灯相应的IO口
    InitKey();             //设置KEY相应的IO口
    
    while(1)
    {
        if(KeyValue == 1)   //切换到LED3、LED2、LED1将倒序流水灯闪烁 
        {  
            LED3 = !LED3;         
            DelayMS(200); 
            LED2 = !LED2;         
            DelayMS(200);            
            LED1 = !LED1;         
            DelayMS(200);
        }
        else // 切换到LED1闪烁效果
        {
            LedOnOrOff(0);
            LED1 = ON;    //点亮LED1      
            DelayMS(500); 
            LED1 = OFF;   //熄灭LED1 
            DelayMS(500);           
        
        }
    }
}