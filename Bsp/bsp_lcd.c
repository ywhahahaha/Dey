#include "bsp_lcd.h"
#include "stdlib.h"
#include "font.h"  
#include "delay.h"
#include "stdio.h"


SRAM_HandleTypeDef TFTSRAM_Handler;                                                 //SRAM句柄(用于控制LCD)

/**************LCD的画笔颜色和背景色****************/  
u32 POINT_COLOR=0xFF000000;		                                                    //画笔颜色
u32 BACK_COLOR =0xFFFFFFFF;  	                                                    //背景色 


_lcd_dev lcddev;                                                                    //LCD控制器
	
void opt_delay(u8 i)
{
	while(i--);
}

//写寄存器函数
//regval:寄存器值
void 
LCD_WR_REG (vu16 regval)
{       
    regval=regval;		                                                            //使用-O2优化的时候,必须插入的延时
	LCD->LCD_REG=regval >> 8;                                                            //写入要写的寄存器序号
    LCD->LCD_REG=regval & 0xff;  
}

//写LCD数据
//data:要写入的值
void LCD_WR_DATA (vu16 data)
{	  
	data=data;			                                                            //使用-O2优化的时候,必须插入的延时
	LCD->LCD_RAM=data >> 8;
    LCD->LCD_RAM=data & 0xff;
}

//读LCD数据
//返回值:读到的值
u16 LCD_RD_DATA (void)
{
    vu16 ram;			                                                            //防止被优化
	ram=LCD->LCD_RAM;	
	return ram;	 
}

//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据
void LCD_WriteReg (u16 LCD_Reg, u16 LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg >> 8;		                                                    //写入要写的寄存器序号
    LCD->LCD_REG = LCD_Reg & 0xff;
	LCD->LCD_RAM = LCD_RegValue >> 8;                                                    //写入数据
    LCD->LCD_RAM = LCD_RegValue & 0xff;                                             //写入数据	   		 
}

//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据
u16 LCD_ReadReg (u16 LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		                                                    //写入要读的寄存器序号
	delay_us(5);		  
	return LCD_RD_DATA();		                                                    //返回读到的值
}

//开始写GRAM
void LCD_WriteRAM_Prepare (void)
{
    LCD->LCD_REG=lcddev.wramcmd >> 8;
    LCD->LCD_REG=lcddev.wramcmd & 0xff;  
}

//LCD写GRAM
//RGB_Code:颜色值
void LCD_WriteRAM (u16 RGB_Code)
{							    
    LCD->LCD_REG=lcddev.wramcmd >> 8;
    LCD->LCD_REG=lcddev.wramcmd & 0xff;
}

//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
u16 LCD_BGR2RGB (u16 c)
{
	u16  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f; 
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
}

//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
u32 LCD_ReadPoint (u16 x,u16 y)
{
 	u16 r=0,g=0,b=0;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	                                //超过了范围,直接返回		   
	LCD_SetCursor(x,y);
    if (lcddev.id == 0X5510) {                                                      //5510 发送读GRAM指令
        LCD_WR_REG(0X2E00);
    } else {                                                                        //其他IC(9341/5310/1963/7789)发送读GRAM指令
        LCD_WR_REG(0X2E);
    }
    
 	r=LCD_RD_DATA();								                                //dummy Read	   
    if (lcddev.id == 0X1963) {                                                      //对1963来说,是真读
        return r;                                                                   //1963直接读就可以
    }
	opt_delay(2);
    r = LCD_RD_DATA();                                                              //实际坐标颜色

    //9341/5310/5510/7789 要分2次读出
	opt_delay(2);
    b = LCD_RD_DATA();
    g = r & 0XFF;                                                                   //对于 9341/5310/5510/7789, 第一次读取的是RG的值,R在前,G在后,各占8位
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));                      // 9341/5310/5510/7789 需要公式转换一下
}

//LCD开启显示
void LCD_DisplayOn (void)
{					   
    if (lcddev.id == 0X5510)                                                        //5510开启显示指令
    {
        LCD_WR_REG(0X2900);                                                         //开启显示
    }
    else                                                                            //9341/5310/1963/7789 等发送开启显示指令
    {
        LCD_WR_REG(0X29);                                                           //开启显示
    }
}

//LCD关闭显示
void LCD_DisplayOff (void)
{	   
    if (lcddev.id == 0X5510)                                                        //5510关闭显示指令
    {
        LCD_WR_REG(0X2800);                                                         //关闭显示
    } else {                                                                        //9341/5310/1963/7789 等发送关闭显示指令   
        LCD_WR_REG(0X28);                                                           //关闭显示
    }
}

//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor (u16 Xpos, u16 Ypos)
{
    if (lcddev.id == 0X1963)
    {
        if (lcddev.dir == 0)                                                        //x坐标需要变换
        {
            Xpos = lcddev.width - 1 - Xpos;
            LCD_WR_REG(lcddev.setxcmd);
            LCD_WR_DATA(0);
            LCD_WR_DATA(0);
            LCD_WR_DATA(Xpos >> 8);
            LCD_WR_DATA(Xpos & 0XFF);
        } else {
            LCD_WR_REG(lcddev.setxcmd);
            LCD_WR_DATA(Xpos >> 8);
            LCD_WR_DATA(Xpos & 0XFF);
            LCD_WR_DATA((lcddev.width - 1) >> 8);
            LCD_WR_DATA((lcddev.width - 1) & 0XFF);
        }

        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_DATA(Ypos & 0XFF);
        LCD_WR_DATA((lcddev.height - 1) >> 8);
        LCD_WR_DATA((lcddev.height - 1) & 0XFF);

    } else if (lcddev.id == 0X5510) {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(Ypos & 0XFF);
    } else {                                                                        //9341/5310/7789等设置坐标
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_REG(lcddev.setycmd);       
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_DATA(Ypos & 0XFF);
    }
}

//设置LCD的自动扫描方向
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9341/5310/5510/1963/7789等IC已经实际测试
//注意:其他函数可能会受到此函数设置的影响(尤其是9341),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
void LCD_Scan_Dir (u8 dir)
{
	u16 regval=0;
	u16 dirreg=0;
	u16 temp;  
    //横屏时，对1963不改变扫描方向, 其他IC改变扫描方向！竖屏时1963改变方向, 其他IC不改变扫描方向
    if ((lcddev.dir == 1 && lcddev.id != 0X1963) || (lcddev.dir == 0 && lcddev.id == 0X1963))
    {
        switch (dir)   //方向转换
        {
            case 0:
                dir = 6;
                break;

            case 1:
                dir = 7;
                break;

            case 2:
                dir = 4;
                break;

            case 3:
                dir = 5;
                break;

            case 4:
                dir = 1;
                break;

            case 5:
                dir = 0;
                break;

            case 6:
                dir = 3;
                break;

            case 7:
                dir = 2;
                break;
        }
    }

    switch (dir)
    {
        case L2R_U2D://从左到右,从上到下
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U://从左到右,从下到上
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D://从右到左,从上到下
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U://从右到左,从下到上
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R://从上到下,从左到右
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L://从上到下,从右到左
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R://从下到上,从左到右
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L://从下到上,从右到左
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

    if (lcddev.id == 0X5510)dirreg = 0X3600;
    else dirreg = 0X36;

    if (lcddev.id == 0X9341 || lcddev.id == 0X7789)   //9341 & 7789 要设置BGR位
    {
        regval |= 0X08;
    }

    LCD_WriteReg(dirreg, regval);

    if (lcddev.id != 0X1963)   //1963不做坐标处理
    {
        if (regval & 0X20)
        {
            if (lcddev.width < lcddev.height)   //交换X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   //交换X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    //设置显示区域(开窗)大小
    if (lcddev.id == 0X5510)
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setxcmd + 2);
        LCD_WR_DATA((lcddev.width - 1) >> 8);
        LCD_WR_REG(lcddev.setxcmd + 3);
        LCD_WR_DATA((lcddev.width - 1) & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setycmd + 2);
        LCD_WR_DATA((lcddev.height - 1) >> 8);
        LCD_WR_REG(lcddev.setycmd + 3);
        LCD_WR_DATA((lcddev.height - 1) & 0XFF);
    }
    else
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(0);
        LCD_WR_DATA(0);
        LCD_WR_DATA((lcddev.width - 1) >> 8);
        LCD_WR_DATA((lcddev.width - 1) & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(0);
        LCD_WR_DATA(0);
        LCD_WR_DATA((lcddev.height - 1) >> 8);
        LCD_WR_DATA((lcddev.height - 1) & 0XFF);
    }
}

//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint (u16 x, u16 y)
{ 
	LCD_SetCursor(x,y);		                                                          //设置光标位置 
	LCD_WriteRAM_Prepare();	                                                          //开始写入GRAM
    LCD->LCD_RAM=POINT_COLOR >> 8;
    LCD->LCD_RAM=POINT_COLOR & 0xff;
}

//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint (u16 x, u16 y, u32 color)
{	   
 if (lcddev.id == 0X5510) {    
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(x >> 8);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(y >> 8);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(y & 0XFF);
    } else if (lcddev.id == 0X1963) {
   
        if (lcddev.dir == 0) {
            x = lcddev.width - 1 - x;
        }

        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(x >> 8);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_DATA(x >> 8);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(y >> 8);
        LCD_WR_DATA(y & 0XFF);
        LCD_WR_DATA(y >> 8);
        LCD_WR_DATA(y & 0XFF);
    } else {                                                                        //9341/5310/7789等设置坐标  
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(x >> 8);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(y >> 8);
        LCD_WR_DATA(y & 0XFF);
    }

    LCD->LCD_REG=lcddev.wramcmd >> 8;
    LCD->LCD_REG=lcddev.wramcmd & 0xff;
    LCD->LCD_RAM= (color) >> 8;
    LCD->LCD_RAM= color & 0xff;
}

//SSD1963 背光设置
//pwm:背光等级,0~100.越大越亮.
void LCD_SSD_BackLightSet (u8 pwm)
{	
	LCD_WR_REG(0xBE);	                                                              //配置PWM输出
	LCD_WR_DATA(0x05);	                                                              //1设置PWM频率
	LCD_WR_DATA(pwm*2.55);                                                            //2设置PWM占空比
	LCD_WR_DATA(0x01);	                                                              //3设置C
	LCD_WR_DATA(0xFF);	                                                              //4设置D
	LCD_WR_DATA(0x00);	                                                              //5设置E
	LCD_WR_DATA(0x00);	                                                              //6设置F
}

//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir (u8 dir)
{
    lcddev.dir = dir;       //竖屏/横屏

    if (dir == 0)           //竖屏
    {
        lcddev.width = 240;
        lcddev.height = 320;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        }
        else if (lcddev.id == 0X1963)
        {
            lcddev.wramcmd = 0X2C;  //设置写入GRAM的指令
            lcddev.setxcmd = 0X2B;  //设置写X坐标指令
            lcddev.setycmd = 0X2A;  //设置写Y坐标指令
            lcddev.width = 480;     //设置宽度480
            lcddev.height = 800;    //设置高度800
        }
        else                        //其他IC, 包括: 9341 / 5310 / 7789等IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310)    //如果是5310 则表示是 320*480分辨率
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }
    }
    else     //横屏
    {
        lcddev.width = 320;
        lcddev.height = 240;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        }
        else if (lcddev.id == 0X1963)
        {
            lcddev.wramcmd = 0X2C;  //设置写入GRAM的指令
            lcddev.setxcmd = 0X2A;  //设置写X坐标指令
            lcddev.setycmd = 0X2B;  //设置写Y坐标指令
            lcddev.width = 800;     //设置宽度800
            lcddev.height = 480;    //设置高度480
        }
        else                        //其他IC, 包括: 9341 / 5310 / 7789等IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310)    //如果是5310 则表示是 320*480分辨率
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    LCD_Scan_Dir(DFT_SCAN_DIR);     //默认扫描方向
}

//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
void LCD_Set_Window (u16 sx, u16 sy, u16 width, u16 height)
{
    u16 twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0X1963 && lcddev.dir != 1) {                                   //1963竖屏特殊处理
    
        sx = lcddev.width - width - sx;
        height = sy + height - 1;
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(sx >> 8);
        LCD_WR_DATA(sx & 0XFF);
        LCD_WR_DATA((sx + width - 1) >> 8);
        LCD_WR_DATA((sx + width - 1) & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(sy >> 8);
        LCD_WR_DATA(sy & 0XFF);
        LCD_WR_DATA(height >> 8);
        LCD_WR_DATA(height & 0XFF);
    } else if (lcddev.id == 0X5510) {    
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(sx >> 8);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(sx & 0XFF);
        LCD_WR_REG(lcddev.setxcmd + 2);
        LCD_WR_DATA(twidth >> 8);
        LCD_WR_REG(lcddev.setxcmd + 3);
        LCD_WR_DATA(twidth & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(sy >> 8);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(sy & 0XFF);
        LCD_WR_REG(lcddev.setycmd + 2);
        LCD_WR_DATA(theight >> 8);
        LCD_WR_REG(lcddev.setycmd + 3);
        LCD_WR_DATA(theight & 0XFF);
    } else {                                                                        //9341/5310/7789/1963横屏 等 设置窗口    
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(sx >> 8);
        LCD_WR_DATA(sx & 0XFF);
        LCD_WR_DATA(twidth >> 8);
        LCD_WR_DATA(twidth & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(sy >> 8);
        LCD_WR_DATA(sy & 0XFF);
        LCD_WR_DATA(theight >> 8);
        LCD_WR_DATA(theight & 0XFF);
    }
}

//SRAM底层驱动，时钟使能，引脚分配
//此函数会被HAL_SRAM_Init()调用
//hsram:SRAM句柄
void HAL_SRAM_MspInit (SRAM_HandleTypeDef *hsram)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_FSMC_CLK_ENABLE();			                                        //使能FSMC时钟
	__HAL_RCC_GPIOD_CLK_ENABLE();			                                        //使能GPIOD时钟
	__HAL_RCC_GPIOE_CLK_ENABLE();			                                        //使能GPIOE时钟
  /** FSMC GPIO Configuration
  PE7   ------> FSMC_D4
  PE8   ------> FSMC_D5
  PE9   ------> FSMC_D6
  PE10   ------> FSMC_D7
  PD14   ------> FSMC_D0
  PD15   ------> FSMC_D1
  PD0   ------> FSMC_D2
  PD1   ------> FSMC_D3
  PD4   ------> FSMC_NOE
  PD5   ------> FSMC_NWE
  */	
	//初始化PD0,1,4,5,8,9,10,14,15
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_Initure.Mode=GPIO_MODE_AF_PP; 		                                        //推挽复用
	GPIO_Initure.Pull=GPIO_PULLUP;			                                        //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;		                                        //高速
	GPIO_Initure.Alternate=GPIO_AF12_FSMC;	                                        //复用为FSMC
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);                                             //初始化
	
	//初始化PE7,8,9,10,11,12,13,14,15
	GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
}


void Set_Dir (u8 dir)
{
	if((dir>>4)%4)
	{
		lcddev.width=320;
		lcddev.height=240;
	} else {
		lcddev.width=240;
		lcddev.height=320;
	}
}

//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试! 
void LCD_Init (void)
{ 	
    lcddev.id = 0x1111;
	GPIO_InitTypeDef GPIO_Initure;
	FSMC_NORSRAM_TimingTypeDef FSMC_ReadWriteTim;
	FSMC_NORSRAM_TimingTypeDef FSMC_WriteTim;
    
	__HAL_RCC_GPIOE_CLK_ENABLE();			                                        //开启GPIOE时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    
	GPIO_Initure.Pin=GPIO_PIN_3;          	                                        //PE3,背光控制
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;                                          //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;                                                  //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;                                             //高速
	HAL_GPIO_Init(GPIOE,&GPIO_Initure); 	
    
	TFTSRAM_Handler.Instance=FSMC_NORSRAM_DEVICE;                
	TFTSRAM_Handler.Extended=FSMC_NORSRAM_EXTENDED_DEVICE;    
    
	TFTSRAM_Handler.Init.NSBank=FSMC_NORSRAM_BANK4;      				            //使用NE4
	TFTSRAM_Handler.Init.DataAddressMux=FSMC_DATA_ADDRESS_MUX_DISABLE; 	            //地址/数据线不复用
	TFTSRAM_Handler.Init.MemoryType=FSMC_MEMORY_TYPE_SRAM;   			            //SRAM
	TFTSRAM_Handler.Init.MemoryDataWidth=FSMC_NORSRAM_MEM_BUS_WIDTH_8;              //8位数据宽度
	TFTSRAM_Handler.Init.BurstAccessMode=FSMC_BURST_ACCESS_MODE_DISABLE;            //是否使能突发访问,仅对同步突发存储器有效,此处未用到
	TFTSRAM_Handler.Init.WaitSignalPolarity=FSMC_WAIT_SIGNAL_POLARITY_LOW;          //等待信号的极性,仅在突发模式访问下有用
	TFTSRAM_Handler.Init.WaitSignalActive=FSMC_WAIT_TIMING_BEFORE_WS;               //存储器是在等待周期之前的一个时钟周期还是等待周期期间使能NWAIT
	TFTSRAM_Handler.Init.WriteOperation=FSMC_WRITE_OPERATION_ENABLE;                //存储器写使能
	TFTSRAM_Handler.Init.WaitSignal=FSMC_WAIT_SIGNAL_DISABLE;                       //等待使能位,此处未用到
	TFTSRAM_Handler.Init.ExtendedMode=FSMC_EXTENDED_MODE_ENABLE;                    //读写使用不同的时序
	TFTSRAM_Handler.Init.AsynchronousWait=FSMC_ASYNCHRONOUS_WAIT_DISABLE;           //是否使能同步传输模式下的等待信号,此处未用到
	TFTSRAM_Handler.Init.WriteBurst=FSMC_WRITE_BURST_DISABLE;                       //禁止突发写
	TFTSRAM_Handler.Init.ContinuousClock=FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
           
	//FMC读时序控制寄存器
	FSMC_ReadWriteTim.AddressSetupTime=0x0F;       	                                //地址建立时间（ADDSET）为16个HCLK 1/168M=6ns*16=96ns
	FSMC_ReadWriteTim.AddressHoldTime=0;
	FSMC_ReadWriteTim.DataSetupTime=60;				                                //数据保存时间为60个HCLK	=6*60=360ns
	FSMC_ReadWriteTim.AccessMode=FSMC_ACCESS_MODE_A;                                //模式A
	//FMC写时序控制寄存器
	FSMC_WriteTim.BusTurnAroundDuration=0;			                                //总线周转阶段持续时间为0，此变量不赋值的话会莫名其妙的自动修改为4。导致程序运行正常
	FSMC_WriteTim.AddressSetupTime=9;          		                                //地址建立时间（ADDSET）为9个HCLK =54ns 
	FSMC_WriteTim.AddressHoldTime=0;
	FSMC_WriteTim.DataSetupTime=8;              	                                //数据保存时间为6ns*9个HCLK=54n
	FSMC_WriteTim.AccessMode=FSMC_ACCESS_MODE_A;                                    //模式A
	HAL_SRAM_Init(&TFTSRAM_Handler,&FSMC_ReadWriteTim,&FSMC_WriteTim);	
    
    GPIO_Initure.Pin=GPIO_PIN_5;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure); 
    
    GPIO_Initure.Pin=GPIO_PIN_4;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);  
	
    LCD_LED=1;                                                                      // LC背光使能
   
    LCD_RESET = 1; 
    osDelay(30);
    LCD_RESET = 0;
    osDelay(50);
    LCD_RESET = 1;
    osDelay(30);
    LCD_CS = 0;
    
    LCD_WR_REG(0XDA00);
    lcddev.id=LCD_RD_DATA();		//读回0X00	 
	LCD_WR_REG(0XDB00);	
	lcddev.id=LCD_RD_DATA();		//读回0X80
	lcddev.id<<=8;	
	LCD_WR_REG(0XDC00);		
	lcddev.id|=LCD_RD_DATA();		//读回0X00
    printf("id=%x\n",lcddev.id);
    lcddev.id =0x5510;
    printf("id=%x\n",lcddev.id);  
    osDelay(100);
    
    LCD_RESET = 1; 
    osDelay(30);
    LCD_RESET = 0;
    osDelay(50);
    LCD_RESET = 1;
    osDelay(30);
    //lcddev.id=0X9341;
	if(lcddev.id==0x5510) {	         //9341初始化
        osDelay(160);
LCD_WR_REG(0xF000);    LCD_WR_DATA(0x55);
LCD_WR_REG(0xF001);    LCD_WR_DATA(0xAA);
LCD_WR_REG(0xF002);    LCD_WR_DATA(0x52);
LCD_WR_REG(0xF003);    LCD_WR_DATA(0x08);
LCD_WR_REG(0xF004);    LCD_WR_DATA(0x01);

LCD_WR_REG(0xBC01);    LCD_WR_DATA(0xA8);
LCD_WR_REG(0xBC02);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xBD01);    LCD_WR_DATA(0xA8);
LCD_WR_REG(0xBD02);    LCD_WR_DATA(0x00);

LCD_WR_REG(0xBE01);    LCD_WR_DATA(0x50);
//#R+
LCD_WR_REG(0xD100);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD101);    LCD_WR_DATA(0x5D);
LCD_WR_REG(0xD102);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD103);    LCD_WR_DATA(0x67);
LCD_WR_REG(0xD104);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD105);    LCD_WR_DATA(0x7A);
LCD_WR_REG(0xD106);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD107);    LCD_WR_DATA(0x8A);
LCD_WR_REG(0xD108);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD109);    LCD_WR_DATA(0x97);
LCD_WR_REG(0xD10A);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD10B);    LCD_WR_DATA(0xB0);
LCD_WR_REG(0xD10C);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD10D);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD10E);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD10F);    LCD_WR_DATA(0xE8);
LCD_WR_REG(0xD110);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD111);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD112);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD113);    LCD_WR_DATA(0x2D);
LCD_WR_REG(0xD114);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD115);    LCD_WR_DATA(0x4F);
LCD_WR_REG(0xD116);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD117);    LCD_WR_DATA(0x82);
LCD_WR_REG(0xD118);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD119);    LCD_WR_DATA(0xAB);
LCD_WR_REG(0xD11A);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD11B);    LCD_WR_DATA(0xAC);
LCD_WR_REG(0xD11C);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD11D);    LCD_WR_DATA(0xD1);
LCD_WR_REG(0xD11E);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD11F);    LCD_WR_DATA(0xF3);
LCD_WR_REG(0xD120);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD121);    LCD_WR_DATA(0x06);
LCD_WR_REG(0xD122);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD123);    LCD_WR_DATA(0x22);
LCD_WR_REG(0xD124);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD125);    LCD_WR_DATA(0x36);
LCD_WR_REG(0xD126);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD127);    LCD_WR_DATA(0x5F);
LCD_WR_REG(0xD128);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD129);    LCD_WR_DATA(0x87);
LCD_WR_REG(0xD12A);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD12B);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD12C);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD12D);    LCD_WR_DATA(0xEF);
LCD_WR_REG(0xD12E);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD12F);    LCD_WR_DATA(0x53);
LCD_WR_REG(0xD130);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD131);    LCD_WR_DATA(0xA4);
LCD_WR_REG(0xD132);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD133);    LCD_WR_DATA(0xCC);
//#G+
LCD_WR_REG(0xD200);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD201);    LCD_WR_DATA(0x5D);
LCD_WR_REG(0xD202);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD203);    LCD_WR_DATA(0x67);
LCD_WR_REG(0xD204);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD205);    LCD_WR_DATA(0x7A);
LCD_WR_REG(0xD206);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD207);    LCD_WR_DATA(0x8A);
LCD_WR_REG(0xD208);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD209);    LCD_WR_DATA(0x97);
LCD_WR_REG(0xD20A);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD20B);    LCD_WR_DATA(0xB0);
LCD_WR_REG(0xD20C);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD20D);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD20E);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD20F);    LCD_WR_DATA(0xE8);
LCD_WR_REG(0xD210);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD211);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD212);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD213);    LCD_WR_DATA(0x2D);
LCD_WR_REG(0xD214);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD215);    LCD_WR_DATA(0x4F);
LCD_WR_REG(0xD216);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD217);    LCD_WR_DATA(0x82);
LCD_WR_REG(0xD218);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD219);    LCD_WR_DATA(0xAB);
LCD_WR_REG(0xD21A);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD21B);    LCD_WR_DATA(0xAC);
LCD_WR_REG(0xD21C);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD21D);    LCD_WR_DATA(0xD1);
LCD_WR_REG(0xD21E);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD21F);    LCD_WR_DATA(0xF3);
LCD_WR_REG(0xD220);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD221);    LCD_WR_DATA(0x06);
LCD_WR_REG(0xD222);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD223);    LCD_WR_DATA(0x22);
LCD_WR_REG(0xD224);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD225);    LCD_WR_DATA(0x36);
LCD_WR_REG(0xD226);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD227);    LCD_WR_DATA(0x5F);
LCD_WR_REG(0xD228);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD229);    LCD_WR_DATA(0x87);
LCD_WR_REG(0xD22A);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD22B);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD22C);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD22D);    LCD_WR_DATA(0xEF);
LCD_WR_REG(0xD22E);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD22F);    LCD_WR_DATA(0x53);
LCD_WR_REG(0xD230);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD231);    LCD_WR_DATA(0xA4);
LCD_WR_REG(0xD232);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD233);    LCD_WR_DATA(0xCC);
///#B+
LCD_WR_REG(0xD300);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD301);    LCD_WR_DATA(0x5D);
LCD_WR_REG(0xD302);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD303);    LCD_WR_DATA(0x67);
LCD_WR_REG(0xD304);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD305);    LCD_WR_DATA(0x7A);
LCD_WR_REG(0xD306);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD307);    LCD_WR_DATA(0x8A);
LCD_WR_REG(0xD308);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD309);    LCD_WR_DATA(0x97);
LCD_WR_REG(0xD30A);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD30B);    LCD_WR_DATA(0xB0);
LCD_WR_REG(0xD30C);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD30D);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD30E);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD30F);    LCD_WR_DATA(0xE8);
LCD_WR_REG(0xD310);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD311);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD312);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD313);    LCD_WR_DATA(0x2D);
LCD_WR_REG(0xD314);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD315);    LCD_WR_DATA(0x4F);
LCD_WR_REG(0xD316);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD317);    LCD_WR_DATA(0x82);
LCD_WR_REG(0xD318);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD319);    LCD_WR_DATA(0xAB);
LCD_WR_REG(0xD31A);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD31B);    LCD_WR_DATA(0xAC);
LCD_WR_REG(0xD31C);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD31D);    LCD_WR_DATA(0xD1);
LCD_WR_REG(0xD31E);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD31F);    LCD_WR_DATA(0xF3);
LCD_WR_REG(0xD320);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD321);    LCD_WR_DATA(0x06);
LCD_WR_REG(0xD322);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD323);    LCD_WR_DATA(0x22);
LCD_WR_REG(0xD324);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD325);    LCD_WR_DATA(0x36);
LCD_WR_REG(0xD326);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD327);    LCD_WR_DATA(0x5F);
LCD_WR_REG(0xD328);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD329);    LCD_WR_DATA(0x87);
LCD_WR_REG(0xD32A);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD32B);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD32C);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD32D);    LCD_WR_DATA(0xEF);
LCD_WR_REG(0xD32E);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD32F);    LCD_WR_DATA(0x53);
LCD_WR_REG(0xD330);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD331);    LCD_WR_DATA(0xA4);
LCD_WR_REG(0xD332);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD333);    LCD_WR_DATA(0xCC);
///#R-
LCD_WR_REG(0xD400);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD401);    LCD_WR_DATA(0x5D);
LCD_WR_REG(0xD402);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD403);    LCD_WR_DATA(0x67);
LCD_WR_REG(0xD404);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD405);    LCD_WR_DATA(0x7A);
LCD_WR_REG(0xD406);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD407);    LCD_WR_DATA(0x8A);
LCD_WR_REG(0xD408);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD409);    LCD_WR_DATA(0x97);
LCD_WR_REG(0xD40A);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD40B);    LCD_WR_DATA(0xB0);
LCD_WR_REG(0xD40C);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD40D);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD40E);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD40F);    LCD_WR_DATA(0xE8);
LCD_WR_REG(0xD410);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD411);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD412);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD413);    LCD_WR_DATA(0x2D);
LCD_WR_REG(0xD414);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD415);    LCD_WR_DATA(0x4F);
LCD_WR_REG(0xD416);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD417);    LCD_WR_DATA(0x82);
LCD_WR_REG(0xD418);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD419);    LCD_WR_DATA(0xAB);
LCD_WR_REG(0xD41A);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD41B);    LCD_WR_DATA(0xAC);
LCD_WR_REG(0xD41C);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD41D);    LCD_WR_DATA(0xD1);
LCD_WR_REG(0xD41E);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD41F);    LCD_WR_DATA(0xF3);
LCD_WR_REG(0xD420);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD421);    LCD_WR_DATA(0x06);
LCD_WR_REG(0xD422);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD423);    LCD_WR_DATA(0x22);
LCD_WR_REG(0xD424);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD425);    LCD_WR_DATA(0x36);
LCD_WR_REG(0xD426);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD427);    LCD_WR_DATA(0x5F);
LCD_WR_REG(0xD428);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD429);    LCD_WR_DATA(0x87);
LCD_WR_REG(0xD42A);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD42B);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD42C);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD42D);    LCD_WR_DATA(0xEF);
LCD_WR_REG(0xD42E);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD42F);    LCD_WR_DATA(0x53);
LCD_WR_REG(0xD430);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD431);    LCD_WR_DATA(0xA4);
LCD_WR_REG(0xD432);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD433);    LCD_WR_DATA(0xCC);
///#G-
LCD_WR_REG(0xD500);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD501);    LCD_WR_DATA(0x5D);
LCD_WR_REG(0xD502);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD503);    LCD_WR_DATA(0x67);
LCD_WR_REG(0xD504);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD505);    LCD_WR_DATA(0x7A);
LCD_WR_REG(0xD506);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD507);    LCD_WR_DATA(0x8A);
LCD_WR_REG(0xD508);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD509);    LCD_WR_DATA(0x97);
LCD_WR_REG(0xD50A);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD50B);    LCD_WR_DATA(0xB0);
LCD_WR_REG(0xD50C);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD50D);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD50E);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD50F);    LCD_WR_DATA(0xE8);
LCD_WR_REG(0xD510);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD511);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD512);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD513);    LCD_WR_DATA(0x2D);
LCD_WR_REG(0xD514);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD515);    LCD_WR_DATA(0x4F);
LCD_WR_REG(0xD516);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD517);    LCD_WR_DATA(0x82);
LCD_WR_REG(0xD518);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD519);    LCD_WR_DATA(0xAB);
LCD_WR_REG(0xD51A);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD51B);    LCD_WR_DATA(0xAC);
LCD_WR_REG(0xD51C);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD51D);    LCD_WR_DATA(0xD1);
LCD_WR_REG(0xD51E);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD51F);    LCD_WR_DATA(0xF3);
LCD_WR_REG(0xD520);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD521);    LCD_WR_DATA(0x06);
LCD_WR_REG(0xD522);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD523);    LCD_WR_DATA(0x22);
LCD_WR_REG(0xD524);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD525);    LCD_WR_DATA(0x36);
LCD_WR_REG(0xD526);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD527);    LCD_WR_DATA(0x5F);
LCD_WR_REG(0xD528);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD529);    LCD_WR_DATA(0x87);
LCD_WR_REG(0xD52A);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD52B);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD52C);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD52D);    LCD_WR_DATA(0xEF);
LCD_WR_REG(0xD52E);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD52F);    LCD_WR_DATA(0x53);
LCD_WR_REG(0xD530);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD531);    LCD_WR_DATA(0xA4);
LCD_WR_REG(0xD532);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD533);    LCD_WR_DATA(0xCC);
///#B-
LCD_WR_REG(0xD600);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD601);    LCD_WR_DATA(0x5D);
LCD_WR_REG(0xD602);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD603);    LCD_WR_DATA(0x67);
LCD_WR_REG(0xD604);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD605);    LCD_WR_DATA(0x7A);
LCD_WR_REG(0xD606);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD607);    LCD_WR_DATA(0x8A);
LCD_WR_REG(0xD608);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD609);    LCD_WR_DATA(0x97);
LCD_WR_REG(0xD60A);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD60B);    LCD_WR_DATA(0xB0);
LCD_WR_REG(0xD60C);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD60D);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD60E);    LCD_WR_DATA(0x00);
LCD_WR_REG(0xD60F);    LCD_WR_DATA(0xE8);
LCD_WR_REG(0xD610);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD611);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD612);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD613);    LCD_WR_DATA(0x2D);
LCD_WR_REG(0xD614);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD615);    LCD_WR_DATA(0x4F);
LCD_WR_REG(0xD616);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD617);    LCD_WR_DATA(0x82);
LCD_WR_REG(0xD618);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD619);    LCD_WR_DATA(0xAB);
LCD_WR_REG(0xD61A);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD61B);    LCD_WR_DATA(0xAC);
LCD_WR_REG(0xD61C);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD61D);    LCD_WR_DATA(0xD1);
LCD_WR_REG(0xD61E);    LCD_WR_DATA(0x01);
LCD_WR_REG(0xD61F);    LCD_WR_DATA(0xF3);
LCD_WR_REG(0xD620);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD621);    LCD_WR_DATA(0x06);
LCD_WR_REG(0xD622);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD623);    LCD_WR_DATA(0x22);
LCD_WR_REG(0xD624);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD625);    LCD_WR_DATA(0x36);
LCD_WR_REG(0xD626);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD627);    LCD_WR_DATA(0x5F);
LCD_WR_REG(0xD628);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD629);    LCD_WR_DATA(0x87);
LCD_WR_REG(0xD62A);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD62B);    LCD_WR_DATA(0xC6);
LCD_WR_REG(0xD62C);    LCD_WR_DATA(0x02);
LCD_WR_REG(0xD62D);    LCD_WR_DATA(0xEF);
LCD_WR_REG(0xD62E);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD62F);    LCD_WR_DATA(0x53);
LCD_WR_REG(0xD630);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD631);    LCD_WR_DATA(0xA4);
LCD_WR_REG(0xD632);    LCD_WR_DATA(0x03);
LCD_WR_REG(0xD633);    LCD_WR_DATA(0xCC);

LCD_WR_REG(0xB000);    LCD_WR_DATA(0x12);
LCD_WR_REG(0xB001);    LCD_WR_DATA(0x12);
LCD_WR_REG(0xB002);    LCD_WR_DATA(0x12);

LCD_WR_REG(0xB100);    LCD_WR_DATA(0x0A);
LCD_WR_REG(0xB101);    LCD_WR_DATA(0x0A);
LCD_WR_REG(0xB102);    LCD_WR_DATA(0x0A);

LCD_WR_REG(0xBA00);    LCD_WR_DATA(0x24);
LCD_WR_REG(0xBA01);    LCD_WR_DATA(0x24);
LCD_WR_REG(0xBA02);    LCD_WR_DATA(0x24);

LCD_WR_REG(0xB900);    LCD_WR_DATA(0x34);
LCD_WR_REG(0xB901);    LCD_WR_DATA(0x34);
LCD_WR_REG(0xB902);    LCD_WR_DATA(0x34);

///#Enable Page0
LCD_WR_REG(0xF000);    LCD_WR_DATA(0x55);
LCD_WR_REG(0xF001);    LCD_WR_DATA(0xAA);
LCD_WR_REG(0xF002);    LCD_WR_DATA(0x52);
LCD_WR_REG(0xF003);    LCD_WR_DATA(0x08);
LCD_WR_REG(0xF004);    LCD_WR_DATA(0x00);
 
///#
LCD_WR_REG(0xB000);    LCD_WR_DATA(0x08);
LCD_WR_REG(0xB100);    LCD_WR_DATA(0xCC);

LCD_WR_REG(0xBC00);    LCD_WR_DATA(0x05);
LCD_WR_REG(0xBC01);    LCD_WR_DATA(0x05);
LCD_WR_REG(0xBC02);    LCD_WR_DATA(0x05);

LCD_WR_REG(0xB800);    LCD_WR_DATA(0x01);

LCD_WR_REG(0xB700);    LCD_WR_DATA(0x55);
LCD_WR_REG(0xB701);    LCD_WR_DATA(0x55);

LCD_WR_REG(0xBD02);    LCD_WR_DATA(0x07);
LCD_WR_REG(0xBD03);    LCD_WR_DATA(0x31);
LCD_WR_REG(0xBE02);    LCD_WR_DATA(0x07);
LCD_WR_REG(0xBE03);    LCD_WR_DATA(0x31);
LCD_WR_REG(0xBF02);    LCD_WR_DATA(0x07);
LCD_WR_REG(0xBF03);    LCD_WR_DATA(0x31);

LCD_WR_REG(0xFF00);    LCD_WR_DATA(0xAA);
LCD_WR_REG(0xFF01);    LCD_WR_DATA(0x55);
LCD_WR_REG(0xFF02);    LCD_WR_DATA(0x25);
LCD_WR_REG(0xFF03);    LCD_WR_DATA(0x01);

LCD_WR_REG(0x3500);    LCD_WR_DATA(0x00);
LCD_WR_REG(0x3a00);    LCD_WR_DATA(0x55);///
LCD_WR_REG(0x3600);    LCD_WR_DATA(0x00);
   	  
LCD_WR_REG(0x1100	); 
delay_ms(160);
LCD_WR_REG(0x2900	); 
	

	}        


    //初始化完成以后,提速
    if(lcddev.id==0X9341||lcddev.id==0X7789||lcddev.id==0X5310
       ||lcddev.id==0X5510||lcddev.id==0X1963)                                      //如果是这几个IC,则设置WR时序为最快
    {
        //重新配置写时序控制寄存器的时序
        FSMC_Bank1E->BWTR[6]&=~(0XF<<0);                                            //地址建立时间(ADDSET)清零
        FSMC_Bank1E->BWTR[6]&=~(0XF<<8);                                            //数据保存时间清零
        FSMC_Bank1E->BWTR[6]|=3<<0;                                                 //地址建立时间(ADDSET)为4个HCLK =24ns
        if(lcddev.id==0X7789)                                                       //7789独立设置,否则摄像头实验可能有问题
        {
            FSMC_Bank1E->BWTR[6]|=3<<8;                                             //数据保存时间(DATAST)为6ns*4个HCLK=24ns
        }
        else
        {
            FSMC_Bank1E->BWTR[6]|=2<<8;                                             //数据保存时间(DATAST)为6ns*3个HCLK=18ns
        }
    } 
	LCD_Display_Dir(1);		                                                        //设为横屏
	LCD_LED=1;				                                                        //点亮背光	
    MainMenu();
    LCD_CS = 1;
}  

//清屏函数
//color:要清屏的填充色
void LCD_Clear (u32 color)
{
	u32 index=0;      
	u32 totalpoint=lcddev.width; 
	totalpoint*=lcddev.height; 			                                            //得到总点数
	LCD_SetCursor(0x00,0x0000);			                                            //设置光标位置 
	LCD_WriteRAM_Prepare();     		                                            //开始写入GRAM	 	  
	for(index=0;index<totalpoint;index++) {           
        LCD->LCD_RAM=color >> 8;
        LCD->LCD_RAM=color & 0xff;
	} 
}

//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Fill (u16 sx,u16 sy, u16 ex, u16 ey, u32 color)
{          
	u16 i,j;
	u16 xlen=0; 
	xlen=ex-sx+1;	 
	for(i=sy;i<=ey;i++)
	{
		LCD_SetCursor(sx,i);      				//设置光标位置 
		LCD_WriteRAM_Prepare();     			//开始写入GRAM	  
        for (j = 0; j < xlen; j++)
        {
            LCD->LCD_RAM=color >> 8;     //设置光标位置
            LCD->LCD_RAM=color & 0xff;
        }
    }
}

//在指定区域内填充指定颜色块
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
//color:要填充的颜色
void LCD_Color_Fill (u16 sx, u16 sy, u16 ex, u16 ey, const uint8_t *color)
{  
	u16 height,width;
	u16 i,j; 
	width=(ex-sx+1)*2; 			                                                    //得到填充的宽度
	height=(ey-sy+1);			                                                    //高度
	for(i=0;i<height;i++) {
		LCD_SetCursor(sx,sy+i);   	                                                //设置光标位置 
		LCD_WriteRAM_Prepare();                                                     //开始写入GRAM
        for (j = 0; j < width; j++) {
            LCD->LCD_RAM=color[i * width + j];
            
        }
    }
}

//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;                                                               //计算坐标增量
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0) {
        incx = 1;                                                                   //设置单步方向
    } else if (delta_x == 0) {
        incx = 0;                                                                   //垂直线
    } else {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0;                                                                   //水平线
    } else {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y) {
        distance = delta_x;                                                         //选取基本增量坐标轴
    } else {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++ ) {                                          //画线输出   
        LCD_DrawPoint(uRow, uCol);                                                  //画点
        xerr += delta_x ;
        yerr += delta_y ;
        
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

//画矩形
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle (u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}

//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);                                                              //判断下个点位置的标志

    while (a <= b) {
        LCD_DrawPoint(x0 + a, y0 - b);                                              //5
        LCD_DrawPoint(x0 + b, y0 - a);                                              //0
        LCD_DrawPoint(x0 + b, y0 + a);                                              //4
        LCD_DrawPoint(x0 + a, y0 + b);                                              //6
        LCD_DrawPoint(x0 - a, y0 + b);                                              //1
        LCD_DrawPoint(x0 - b, y0 + a);
        LCD_DrawPoint(x0 - a, y0 - b);                                              //2
        LCD_DrawPoint(x0 - b, y0 - a);                                              //7
        a++;

        //使用Bresenham算法画圆
        if (di < 0) {
            di += 4 * a + 6;
        } else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24/32
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar (u16 x, u16 y, u8 num, u8 size, u8 mode)
{  							  
    u8 temp,t1,t;
	u16 y0=y;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);		                                //得到字体一个字符对应点阵集所占的字节数	
 	num=num-' ';                                                                    //得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）
	for(t=0;t<csize;t++) {   
		if (size==12) {
            temp=asc2_1206[num][t]; 	 	                                        //调用1206字体
        } else if (size==16) {
            temp=asc2_1608[num][t];	                                                //调用1608字体
        } else if (size==24) {
            temp=asc2_2412[num][t];	                                                //调用2412字体
        } else if (size==32) {
            temp=asc2_3216[num][t];	                                                //调用3216字体
        } else {
            return;								                                    //没有的字库
        }
		for(t1=0;t1<8;t1++) {			    
			if (temp&0x80) {
                LCD_Fast_DrawPoint(x,y,POINT_COLOR);
            } else if (mode==0) {
                LCD_Fast_DrawPoint(x,y,BACK_COLOR);
            }
			temp<<=1;
			y++;
			if (y>=lcddev.height) {
                return;		//超区域了
            }
			if((y-y0)==size) {
				y=y0;
				x++;
				if(x>=lcddev.width) {
                    return;	//超区域了
                }
				break;
			}
		}
	}
}

//m^n函数
//返回值:m^n次方.
u32 LCD_Pow (u8 m,u8 n)
{
	u32 result=1;	 
	while (n--) {
        result*=m;
    }
	return result;
}

//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色
//num:数值(0~4294967295);
void LCD_ShowNum (u16 x, u16 y, u32 num, u8 len, u8 size)
{
    u8 t, temp;
    u8 enshow = 0;

    for (t = 0; t < len; t++) {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                LCD_ShowChar(x + (size / 2)*t, y, ' ', size, 0);
                continue;
            } else {
                enshow = 1;
            }
        }
        LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, 0);
    }
}

//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum (u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode)
{  
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++) {
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1)) {
			if(temp==0) {
				if (mode&0X80) {
                    LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);
                } else {
                    LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);
                }
 				continue;
			} else {
                enshow=1;
            }	 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
	}
}

//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址
void LCD_ShowString (u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)
{
    u8 x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')) {                                            //判断是不是非法字符!   
        if (x >= width) {
            x = x0;
            y += size;
        }

        if (y >= height) {
            break; //退出
        }

        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }  
}

void Chinese_Show_one (uint16_t x, uint16_t y, uint16_t num, u8 size, u8 mode)
{
    uint16_t temp,t,t1;
    uint16_t y0=y;
    uint16_t csize=(size/8 + ((size%8)?1:0)) * size;                                      //16/8+1
    for(t=0;t<csize;t++) {  
        if(size==16) {     
            temp = Chinese_16[num][t];
        } else if (size == 24) { 
            temp = Chinese_24[num][t];
        } else if (size == 20) { 
            temp = Chinese_20[num][t];
        } else if (size == 34) {
            temp = Chinese_34[num][t];
        } else  {
            return;
        }
        for(t1=0;t1<8;t1++) {
            if(temp&0x80) { 
                LCD_Fast_DrawPoint(x,y,POINT_COLOR);
            }
            else { 
                LCD_Fast_DrawPoint(x,y,BACK_COLOR);
            }
            temp<<=1;
            y++;
			if (y>=lcddev.height) {
                return;		//超区域了
            }
            if((y-y0)==size) {
                y=y0;
                x++;
				if(x>=lcddev.width) {
                    return;	//超区域了
                }
                break;
            } 
         }  
     }  
 }

void Image_Show (uint16_t x, uint16_t y, uint16_t num, u8 size, u8 mode)
{
    uint16_t temp,t,t1;
    uint16_t y0=y;
    uint16_t csize=(size/8 + ((size%8)?1:0)) * size;                                      //16/8+1
    for(t=0;t<csize;t++) {  
        if(size==40) {     
            temp = Image_40[num][t];
        }  else if (size == 50) {
            temp = Image_50[num][t];
        }  else  {
            return;
        }
        for(t1=0;t1<8;t1++) {
            if(temp&0x80) { 
                LCD_Fast_DrawPoint(x,y,POINT_COLOR);
            }
            else { 
                LCD_Fast_DrawPoint(x,y,BACK_COLOR);
            }
            temp<<=1;
            y++;
			if (y>=lcddev.height) {
                return;		//超区域了
            }
            if((y-y0)==size) {
                y=y0;
                x++;
				if(x>=lcddev.width) {
                    return;	//超区域了
                }
                break;
            } 
         }  
     }  
 }














