#include "main.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "usmart.h"
#include "touch.h"
#include "sram.h"
#include "tinyml.h"
/************************************************
 ALIENTEK 探索者STM32F407开发板 实验28
 触摸屏实验-HAL库函数版
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//清空屏幕并在右上角显示"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//清屏   
  	POINT_COLOR=RED;//设置画笔红色
}

//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}

//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  

//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  

//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

// 把 28x28 的 int8 输入放大显示到 LCD 上，用于调试
// 起点 (x0, y0)，每个像素放大 8 倍，28*8 = 224，能放得下
void TP_Buf_Show_28x28(int8_t *q_input, u16 x0, u16 y0)
{
    for (int y = 0; y < 28; y++) {
        for (int x = 0; x < 28; x++) {
            u8 v = (u8)(q_input[y * 28 + x] + 128);   // 还原到 0~255
            u16 color;
            if (v > 200)      color = WHITE;
            else if (v > 100) color = GRAY;
            else if (v > 30)  color = DARKBLUE;
            else              color = BLACK;
            LCD_Fill(x0 + x*8, y0 + y*8, x0 + x*8 + 7, y0 + y*8 + 7, color);
        }
    }
}

//5个触控点的颜色(电容触摸屏用)												 
const u16 POINT_COLOR_TBL[5]={RED,GREEN,BLUE,BROWN,GRED};  
//电容触摸屏测试函数
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	
	u8 key;  	    
 	u16 lastpos[5][2];		//最后一次的数据 
	while(1)
	{
		key=KEY_Scan(0);
		tp_dev.scan(0);
		if(key==KEY1_PRES)	//KEY0按下,则执行校准程序
		{
			LCD_Clear(WHITE);	//清屏 
			Load_Drow_Dialog();
			LCD_ShowString(30,170,240,16,16,(u8*)"KEY1:TEST TINYML AND CLEAN");
			delay_ms(10);

			// KEY1 触发：冻结位图 → 进入 TinyML 处理
            static int8_t q_input[28*28];
			int8_t q_output[10];

			// 1. 位图缩放到 28x28，得到 u8 灰度图（0~255）
			// 2. 转 int8：你的模型输入 scale=1.0, zero_point=-128，所以 q = u8 - 128
			TP_Buf_To_28x28(q_input);
			TP_Buf_Show_28x28(q_input, 240, 100);   // 显示在屏幕右侧

			u32 total = 0;
			int y,x;
			for (y = 0; y < TP_BUF_H; y++)
				for (x = 0; x < TP_BUF_W; x++)
					if (TP_BUF_GET(x, y)) total++;
			LCD_ShowNum(30, 250, total, 5, 16);

			// 3. 推理
			if (tinyml_predict(q_input, q_output) != 0){
				POINT_COLOR = RED;
				LCD_ShowString(30, 190, 240, 16, 16, (u8*)"Infer failed");
			} else {
				// 4. 取最大下标
				int max_idx = 0;
				for (int i = 1; i < 10; i++)
					if (q_output[i] > q_output[max_idx]) max_idx = i;

				POINT_COLOR = RED;
				LCD_ShowString(30, 190, 240, 16, 16, (u8*)"Result:");
				LCD_ShowNum(30 + 16*7, 190, max_idx, 1, 16);
			}

			// 显示结果之后
    		TP_BUF_CLEAR();   // 清空位图，准备下一轮
			delay_ms(100);
		}
		for(t=0;t<5;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
                //printf("X坐标:%d,Y坐标:%d\r\n",tp_dev.x[0],tp_dev.y[0]);
				if (tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
					if (lastpos[t][0] == 0XFFFF) {
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]); //画线

					// ===== 测试：显示坐标 =====
					u16 save_color = POINT_COLOR;
					POINT_COLOR = BLACK;
					LCD_ShowString(10, 400, 200, 16, 16, (u8*)"from:");
					LCD_ShowNum(60, 400, lastpos[t][0], 3, 16);
					LCD_ShowNum(100, 400, lastpos[t][1], 3, 16);
					LCD_ShowString(10, 420, 200, 16, 16, (u8*)"to:  ");
					LCD_ShowNum(60, 420, tp_dev.x[t], 3, 16);
					LCD_ShowNum(100, 420, tp_dev.y[t], 3, 16);
					POINT_COLOR = save_color;
					// ===== 测试结束 =====

					TP_Buf_DrawLine(lastpos[t][0], lastpos[t][1], tp_dev.x[t], tp_dev.y[t]);
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	
}

int main(void)
{ 
	
    HAL_Init();                   	//初始化HAL库    
    Stm32_Clock_Init(336,8,2,7);  	//设置时钟,168Mhz
	delay_init(168);               	//初始化延时函数
	uart_init(115200);             	//初始化USART
	usmart_dev.init(84); 		    //初始化USMART
	LED_Init();						//初始化LED	
	KEY_Init();						//初始化KEY
 	LCD_Init();           			//初始化LCD
	tp_dev.init();				    //触摸屏初始化 
	SRAM_Init();					//初始化外部SRAM 

	TP_BUF_CLEAR();                 // 手动清零位图（外部 SRAM 上电内容随机）
	
  	POINT_COLOR=RED;
	LCD_ShowString(30,50,200,16,16,(u8*)"Explorer STM32F4");	
	LCD_ShowString(30,70,240,16,16,(u8*)"TOUCH TEST AND TINYML TEST");	
	LCD_ShowString(30,90,200,16,16,(u8*)"USING ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,(u8*)"2026/10/20");	 		

	LCD_ShowString(30,170,240,16,16,(u8*)"KEY1:TEST TINYML AND CLEAN");
	delay_ms(1000);	

	if(tp_dev.touchtype&0X80) ctp_test(); //电容屏测试
}
