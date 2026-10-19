### 实验器材:
- 阿波罗STM32F429开发板
	
### 实验目的:
- 学习STM32 tinyml
	
### 硬件资源:
1. ALIENTEK 2.8/3.5/4.3/7寸LCD模块(包括MCU屏和RGB屏,都支持) 
2. 按键KEY0(PH3)/KEY1(PH2)/KEY2(PH1)

	
### 实验现象:
	使用了外部SRAM、LCD、Touch

	KEY0：检测外部SRAM
	KEY1：画0~9数字
	KEY2：内部CNN网络分类，在lcd上显示你画的数字
	
注意事项:
1. 4.3寸和7寸屏需要比较大电流,USB供电可能不足,请用外部电源适配器(推荐外接12V 1A电源).
2. 本例程在LCD_Init函数里面(在lcd.c),用到了printf,如果不初始化串口1,将导致液晶无法显示!!  