#include "bsp_lcd.h"
#include "stdlib.h"
#include "font.h"  
#include "delay.h"
#include "stdio.h"


SRAM_HandleTypeDef TFTSRAM_Handler;                                                 //SRAM���(���ڿ���LCD)

/**************LCD�Ļ�����ɫ�ͱ���ɫ****************/  
u32 POINT_COLOR=0xFF000000;		                                                    //������ɫ
u32 BACK_COLOR =0xFFFFFFFF;  	                                                    //����ɫ 


_lcd_dev lcddev;                                                                    //LCD������
	
void opt_delay(u8 i)
{
	while(i--);
}

//д�Ĵ�������
//regval:�Ĵ���ֵ
void 
LCD_WR_REG (vu16 regval)
{       
    regval=regval;		                                                            //ʹ��-O2�Ż���ʱ��,����������ʱ
	LCD->LCD_REG=regval >> 8;                                                            //д��Ҫд�ļĴ������
    LCD->LCD_REG=regval & 0xff;  
}

//дLCD����
//data:Ҫд���ֵ
void LCD_WR_DATA (vu16 data)
{	  
	data=data;			                                                            //ʹ��-O2�Ż���ʱ��,����������ʱ
	LCD->LCD_RAM=data >> 8;
    LCD->LCD_RAM=data & 0xff;
}

//��LCD����
//����ֵ:������ֵ
u16 LCD_RD_DATA (void)
{
    vu16 ram;			                                                            //��ֹ���Ż�
	ram=LCD->LCD_RAM;	
	return ram;	 
}

//д�Ĵ���
//LCD_Reg:�Ĵ�����ַ
//LCD_RegValue:Ҫд�������
void LCD_WriteReg (u16 LCD_Reg, u16 LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg >> 8;		                                                    //д��Ҫд�ļĴ������
    LCD->LCD_REG = LCD_Reg & 0xff;
	LCD->LCD_RAM = LCD_RegValue >> 8;                                                    //д������
    LCD->LCD_RAM = LCD_RegValue & 0xff;                                             //д������	   		 
}

//���Ĵ���
//LCD_Reg:�Ĵ�����ַ
//����ֵ:����������
u16 LCD_ReadReg (u16 LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		                                                    //д��Ҫ���ļĴ������
	delay_us(5);		  
	return LCD_RD_DATA();		                                                    //���ض�����ֵ
}

//��ʼдGRAM
void LCD_WriteRAM_Prepare (void)
{
    LCD->LCD_REG=lcddev.wramcmd >> 8;
    LCD->LCD_REG=lcddev.wramcmd & 0xff;  
}

//LCDдGRAM
//RGB_Code:��ɫֵ
void LCD_WriteRAM (u16 RGB_Code)
{							    
    LCD->LCD_REG=lcddev.wramcmd >> 8;
    LCD->LCD_REG=lcddev.wramcmd & 0xff;
}

//��ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ��
//ͨ���ú���ת��
//c:GBR��ʽ����ɫֵ
//����ֵ��RGB��ʽ����ɫֵ
u16 LCD_BGR2RGB (u16 c)
{
	u16  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f; 
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
}

//��ȡ��ĳ�����ɫֵ	 
//x,y:����
//����ֵ:�˵����ɫ
u32 LCD_ReadPoint (u16 x,u16 y)
{
 	u16 r=0,g=0,b=0;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	                                //�����˷�Χ,ֱ�ӷ���		   
	LCD_SetCursor(x,y);
    if (lcddev.id == 0X5510) {                                                      //5510 ���Ͷ�GRAMָ��
        LCD_WR_REG(0X2E00);
    } else {                                                                        //����IC(9341/5310/1963/7789)���Ͷ�GRAMָ��
        LCD_WR_REG(0X2E);
    }
    
 	r=LCD_RD_DATA();								                                //dummy Read	   
    if (lcddev.id == 0X1963) {                                                      //��1963��˵,�����
        return r;                                                                   //1963ֱ�Ӷ��Ϳ���
    }
	opt_delay(2);
    r = LCD_RD_DATA();                                                              //ʵ��������ɫ

    //9341/5310/5510/7789 Ҫ��2�ζ���
	opt_delay(2);
    b = LCD_RD_DATA();
    g = r & 0XFF;                                                                   //���� 9341/5310/5510/7789, ��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));                      // 9341/5310/5510/7789 ��Ҫ��ʽת��һ��
}

//LCD������ʾ
void LCD_DisplayOn (void)
{					   
    if (lcddev.id == 0X5510)                                                        //5510������ʾָ��
    {
        LCD_WR_REG(0X2900);                                                         //������ʾ
    }
    else                                                                            //9341/5310/1963/7789 �ȷ��Ϳ�����ʾָ��
    {
        LCD_WR_REG(0X29);                                                           //������ʾ
    }
}

//LCD�ر���ʾ
void LCD_DisplayOff (void)
{	   
    if (lcddev.id == 0X5510)                                                        //5510�ر���ʾָ��
    {
        LCD_WR_REG(0X2800);                                                         //�ر���ʾ
    } else {                                                                        //9341/5310/1963/7789 �ȷ��͹ر���ʾָ��   
        LCD_WR_REG(0X28);                                                           //�ر���ʾ
    }
}

//���ù��λ��
//Xpos:������
//Ypos:������
void LCD_SetCursor (u16 Xpos, u16 Ypos)
{
    if (lcddev.id == 0X1963)
    {
        if (lcddev.dir == 0)                                                        //x������Ҫ�任
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
    } else {                                                                        //9341/5310/7789����������
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_REG(lcddev.setycmd);       
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_DATA(Ypos & 0XFF);
    }
}

//����LCD���Զ�ɨ�跽��
//dir:0~7,����8������(���嶨���lcd.h)
//9341/5310/5510/1963/7789��IC�Ѿ�ʵ�ʲ���
//ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341),
//����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
void LCD_Scan_Dir (u8 dir)
{
	u16 regval=0;
	u16 dirreg=0;
	u16 temp;  
    //����ʱ����1963���ı�ɨ�跽��, ����IC�ı�ɨ�跽������ʱ1963�ı䷽��, ����IC���ı�ɨ�跽��
    if ((lcddev.dir == 1 && lcddev.id != 0X1963) || (lcddev.dir == 0 && lcddev.id == 0X1963))
    {
        switch (dir)   //����ת��
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
        case L2R_U2D://������,���ϵ���
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U://������,���µ���
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D://���ҵ���,���ϵ���
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U://���ҵ���,���µ���
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R://���ϵ���,������
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L://���ϵ���,���ҵ���
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R://���µ���,������
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L://���µ���,���ҵ���
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

    if (lcddev.id == 0X5510)dirreg = 0X3600;
    else dirreg = 0X36;

    if (lcddev.id == 0X9341 || lcddev.id == 0X7789)   //9341 & 7789 Ҫ����BGRλ
    {
        regval |= 0X08;
    }

    LCD_WriteReg(dirreg, regval);

    if (lcddev.id != 0X1963)   //1963�������괦��
    {
        if (regval & 0X20)
        {
            if (lcddev.width < lcddev.height)   //����X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   //����X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    //������ʾ����(����)��С
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

//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint (u16 x, u16 y)
{ 
	LCD_SetCursor(x,y);		                                                          //���ù��λ�� 
	LCD_WriteRAM_Prepare();	                                                          //��ʼд��GRAM
    LCD->LCD_RAM=POINT_COLOR >> 8;
    LCD->LCD_RAM=POINT_COLOR & 0xff;
}

//���ٻ���
//x,y:����
//color:��ɫ
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
    } else {                                                                        //9341/5310/7789����������  
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

//SSD1963 ��������
//pwm:����ȼ�,0~100.Խ��Խ��.
void LCD_SSD_BackLightSet (u8 pwm)
{	
	LCD_WR_REG(0xBE);	                                                              //����PWM���
	LCD_WR_DATA(0x05);	                                                              //1����PWMƵ��
	LCD_WR_DATA(pwm*2.55);                                                            //2����PWMռ�ձ�
	LCD_WR_DATA(0x01);	                                                              //3����C
	LCD_WR_DATA(0xFF);	                                                              //4����D
	LCD_WR_DATA(0x00);	                                                              //5����E
	LCD_WR_DATA(0x00);	                                                              //6����F
}

//����LCD��ʾ����
//dir:0,������1,����
void LCD_Display_Dir (u8 dir)
{
    lcddev.dir = dir;       //����/����

    if (dir == 0)           //����
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
            lcddev.wramcmd = 0X2C;  //����д��GRAM��ָ��
            lcddev.setxcmd = 0X2B;  //����дX����ָ��
            lcddev.setycmd = 0X2A;  //����дY����ָ��
            lcddev.width = 480;     //���ÿ��480
            lcddev.height = 800;    //���ø߶�800
        }
        else                        //����IC, ����: 9341 / 5310 / 7789��IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310)    //�����5310 ���ʾ�� 320*480�ֱ���
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }
    }
    else     //����
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
            lcddev.wramcmd = 0X2C;  //����д��GRAM��ָ��
            lcddev.setxcmd = 0X2A;  //����дX����ָ��
            lcddev.setycmd = 0X2B;  //����дY����ָ��
            lcddev.width = 800;     //���ÿ��800
            lcddev.height = 480;    //���ø߶�480
        }
        else                        //����IC, ����: 9341 / 5310 / 7789��IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310)    //�����5310 ���ʾ�� 320*480�ֱ���
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    LCD_Scan_Dir(DFT_SCAN_DIR);     //Ĭ��ɨ�跽��
}

//���ô���,���Զ����û������굽�������Ͻ�(sx,sy).
//sx,sy:������ʼ����(���Ͻ�)
//width,height:���ڿ�Ⱥ͸߶�,�������0!!
//�����С:width*height.
void LCD_Set_Window (u16 sx, u16 sy, u16 width, u16 height)
{
    u16 twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0X1963 && lcddev.dir != 1) {                                   //1963�������⴦��
    
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
    } else {                                                                        //9341/5310/7789/1963���� �� ���ô���    
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

//SRAM�ײ�������ʱ��ʹ�ܣ����ŷ���
//�˺����ᱻHAL_SRAM_Init()����
//hsram:SRAM���
void HAL_SRAM_MspInit (SRAM_HandleTypeDef *hsram)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_FSMC_CLK_ENABLE();			                                        //ʹ��FSMCʱ��
	__HAL_RCC_GPIOD_CLK_ENABLE();			                                        //ʹ��GPIODʱ��
	__HAL_RCC_GPIOE_CLK_ENABLE();			                                        //ʹ��GPIOEʱ��
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
	//��ʼ��PD0,1,4,5,8,9,10,14,15
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_Initure.Mode=GPIO_MODE_AF_PP; 		                                        //���츴��
	GPIO_Initure.Pull=GPIO_PULLUP;			                                        //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;		                                        //����
	GPIO_Initure.Alternate=GPIO_AF12_FSMC;	                                        //����ΪFSMC
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);                                             //��ʼ��
	
	//��ʼ��PE7,8,9,10,11,12,13,14,15
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

//��ʼ��lcd
//�ó�ʼ���������Գ�ʼ������ILI93XXҺ��,�������������ǻ���ILI9320��!!!
//�������ͺŵ�����оƬ��û�в���! 
void LCD_Init (void)
{ 	
    lcddev.id = 0x1111;
	GPIO_InitTypeDef GPIO_Initure;
	FSMC_NORSRAM_TimingTypeDef FSMC_ReadWriteTim;
	FSMC_NORSRAM_TimingTypeDef FSMC_WriteTim;
    
	__HAL_RCC_GPIOE_CLK_ENABLE();			                                        //����GPIOEʱ��
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    
	GPIO_Initure.Pin=GPIO_PIN_3;          	                                        //PE3,�������
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;                                          //�������
	GPIO_Initure.Pull=GPIO_PULLUP;                                                  //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;                                             //����
	HAL_GPIO_Init(GPIOE,&GPIO_Initure); 	
    
	TFTSRAM_Handler.Instance=FSMC_NORSRAM_DEVICE;                
	TFTSRAM_Handler.Extended=FSMC_NORSRAM_EXTENDED_DEVICE;    
    
	TFTSRAM_Handler.Init.NSBank=FSMC_NORSRAM_BANK4;      				            //ʹ��NE4
	TFTSRAM_Handler.Init.DataAddressMux=FSMC_DATA_ADDRESS_MUX_DISABLE; 	            //��ַ/�����߲�����
	TFTSRAM_Handler.Init.MemoryType=FSMC_MEMORY_TYPE_SRAM;   			            //SRAM
	TFTSRAM_Handler.Init.MemoryDataWidth=FSMC_NORSRAM_MEM_BUS_WIDTH_8;              //8λ���ݿ��
	TFTSRAM_Handler.Init.BurstAccessMode=FSMC_BURST_ACCESS_MODE_DISABLE;            //�Ƿ�ʹ��ͻ������,����ͬ��ͻ���洢����Ч,�˴�δ�õ�
	TFTSRAM_Handler.Init.WaitSignalPolarity=FSMC_WAIT_SIGNAL_POLARITY_LOW;          //�ȴ��źŵļ���,����ͻ��ģʽ����������
	TFTSRAM_Handler.Init.WaitSignalActive=FSMC_WAIT_TIMING_BEFORE_WS;               //�洢�����ڵȴ�����֮ǰ��һ��ʱ�����ڻ��ǵȴ������ڼ�ʹ��NWAIT
	TFTSRAM_Handler.Init.WriteOperation=FSMC_WRITE_OPERATION_ENABLE;                //�洢��дʹ��
	TFTSRAM_Handler.Init.WaitSignal=FSMC_WAIT_SIGNAL_DISABLE;                       //�ȴ�ʹ��λ,�˴�δ�õ�
	TFTSRAM_Handler.Init.ExtendedMode=FSMC_EXTENDED_MODE_ENABLE;                    //��дʹ�ò�ͬ��ʱ��
	TFTSRAM_Handler.Init.AsynchronousWait=FSMC_ASYNCHRONOUS_WAIT_DISABLE;           //�Ƿ�ʹ��ͬ������ģʽ�µĵȴ��ź�,�˴�δ�õ�
	TFTSRAM_Handler.Init.WriteBurst=FSMC_WRITE_BURST_DISABLE;                       //��ֹͻ��д
	TFTSRAM_Handler.Init.ContinuousClock=FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
           
	//FMC��ʱ����ƼĴ���
	FSMC_ReadWriteTim.AddressSetupTime=0x0F;       	                                //��ַ����ʱ�䣨ADDSET��Ϊ16��HCLK 1/168M=6ns*16=96ns
	FSMC_ReadWriteTim.AddressHoldTime=0;
	FSMC_ReadWriteTim.DataSetupTime=60;				                                //���ݱ���ʱ��Ϊ60��HCLK	=6*60=360ns
	FSMC_ReadWriteTim.AccessMode=FSMC_ACCESS_MODE_A;                                //ģʽA
	//FMCдʱ����ƼĴ���
	FSMC_WriteTim.BusTurnAroundDuration=0;			                                //������ת�׶γ���ʱ��Ϊ0���˱�������ֵ�Ļ���Ī��������Զ��޸�Ϊ4�����³�����������
	FSMC_WriteTim.AddressSetupTime=9;          		                                //��ַ����ʱ�䣨ADDSET��Ϊ9��HCLK =54ns 
	FSMC_WriteTim.AddressHoldTime=0;
	FSMC_WriteTim.DataSetupTime=8;              	                                //���ݱ���ʱ��Ϊ6ns*9��HCLK=54n
	FSMC_WriteTim.AccessMode=FSMC_ACCESS_MODE_A;                                    //ģʽA
	HAL_SRAM_Init(&TFTSRAM_Handler,&FSMC_ReadWriteTim,&FSMC_WriteTim);	
    
    GPIO_Initure.Pin=GPIO_PIN_5;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure); 
    
    GPIO_Initure.Pin=GPIO_PIN_4;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);  
	
    LCD_LED=1;                                                                      // LC����ʹ��
   
    LCD_RESET = 1; 
    osDelay(30);
    LCD_RESET = 0;
    osDelay(50);
    LCD_RESET = 1;
    osDelay(30);
    LCD_CS = 0;
    
    LCD_WR_REG(0XDA00);
    lcddev.id=LCD_RD_DATA();		//����0X00	 
	LCD_WR_REG(0XDB00);	
	lcddev.id=LCD_RD_DATA();		//����0X80
	lcddev.id<<=8;	
	LCD_WR_REG(0XDC00);		
	lcddev.id|=LCD_RD_DATA();		//����0X00
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
	if(lcddev.id==0x5510) {	         //9341��ʼ��
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


    //��ʼ������Ժ�,����
    if(lcddev.id==0X9341||lcddev.id==0X7789||lcddev.id==0X5310
       ||lcddev.id==0X5510||lcddev.id==0X1963)                                      //������⼸��IC,������WRʱ��Ϊ���
    {
        //��������дʱ����ƼĴ�����ʱ��
        FSMC_Bank1E->BWTR[6]&=~(0XF<<0);                                            //��ַ����ʱ��(ADDSET)����
        FSMC_Bank1E->BWTR[6]&=~(0XF<<8);                                            //���ݱ���ʱ������
        FSMC_Bank1E->BWTR[6]|=3<<0;                                                 //��ַ����ʱ��(ADDSET)Ϊ4��HCLK =24ns
        if(lcddev.id==0X7789)                                                       //7789��������,��������ͷʵ�����������
        {
            FSMC_Bank1E->BWTR[6]|=3<<8;                                             //���ݱ���ʱ��(DATAST)Ϊ6ns*4��HCLK=24ns
        }
        else
        {
            FSMC_Bank1E->BWTR[6]|=2<<8;                                             //���ݱ���ʱ��(DATAST)Ϊ6ns*3��HCLK=18ns
        }
    } 
	LCD_Display_Dir(1);		                                                        //��Ϊ����
	LCD_LED=1;				                                                        //��������	
    MainMenu();
    LCD_CS = 1;
}  

//��������
//color:Ҫ���������ɫ
void LCD_Clear (u32 color)
{
	u32 index=0;      
	u32 totalpoint=lcddev.width; 
	totalpoint*=lcddev.height; 			                                            //�õ��ܵ���
	LCD_SetCursor(0x00,0x0000);			                                            //���ù��λ�� 
	LCD_WriteRAM_Prepare();     		                                            //��ʼд��GRAM	 	  
	for(index=0;index<totalpoint;index++) {           
        LCD->LCD_RAM=color >> 8;
        LCD->LCD_RAM=color & 0xff;
	} 
}

//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill (u16 sx,u16 sy, u16 ex, u16 ey, u32 color)
{          
	u16 i,j;
	u16 xlen=0; 
	xlen=ex-sx+1;	 
	for(i=sy;i<=ey;i++)
	{
		LCD_SetCursor(sx,i);      				//���ù��λ�� 
		LCD_WriteRAM_Prepare();     			//��ʼд��GRAM	  
        for (j = 0; j < xlen; j++)
        {
            LCD->LCD_RAM=color >> 8;     //���ù��λ��
            LCD->LCD_RAM=color & 0xff;
        }
    }
}

//��ָ�����������ָ����ɫ��
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)
//color:Ҫ������ɫ
void LCD_Color_Fill (u16 sx, u16 sy, u16 ex, u16 ey, const uint8_t *color)
{  
	u16 height,width;
	u16 i,j; 
	width=(ex-sx+1)*2; 			                                                    //�õ����Ŀ��
	height=(ey-sy+1);			                                                    //�߶�
	for(i=0;i<height;i++) {
		LCD_SetCursor(sx,sy+i);   	                                                //���ù��λ�� 
		LCD_WriteRAM_Prepare();                                                     //��ʼд��GRAM
        for (j = 0; j < width; j++) {
            LCD->LCD_RAM=color[i * width + j];
            
        }
    }
}

//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;                                                               //������������
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0) {
        incx = 1;                                                                   //���õ�������
    } else if (delta_x == 0) {
        incx = 0;                                                                   //��ֱ��
    } else {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0;                                                                   //ˮƽ��
    } else {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y) {
        distance = delta_x;                                                         //ѡȡ��������������
    } else {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++ ) {                                          //�������   
        LCD_DrawPoint(uRow, uCol);                                                  //����
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

//������
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle (u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}

//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);                                                              //�ж��¸���λ�õı�־

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

        //ʹ��Bresenham�㷨��Բ
        if (di < 0) {
            di += 4 * a + 6;
        } else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16/24/32
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar (u16 x, u16 y, u8 num, u8 size, u8 mode)
{  							  
    u8 temp,t1,t;
	u16 y0=y;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);		                                //�õ�����һ���ַ���Ӧ������ռ���ֽ���	
 	num=num-' ';                                                                    //�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩
	for(t=0;t<csize;t++) {   
		if (size==12) {
            temp=asc2_1206[num][t]; 	 	                                        //����1206����
        } else if (size==16) {
            temp=asc2_1608[num][t];	                                                //����1608����
        } else if (size==24) {
            temp=asc2_2412[num][t];	                                                //����2412����
        } else if (size==32) {
            temp=asc2_3216[num][t];	                                                //����3216����
        } else {
            return;								                                    //û�е��ֿ�
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
                return;		//��������
            }
			if((y-y0)==size) {
				y=y0;
				x++;
				if(x>=lcddev.width) {
                    return;	//��������
                }
				break;
			}
		}
	}
}

//m^n����
//����ֵ:m^n�η�.
u32 LCD_Pow (u8 m,u8 n)
{
	u32 result=1;	 
	while (n--) {
        result*=m;
    }
	return result;
}

//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ
//num:��ֵ(0~4294967295);
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

//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
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

//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ
void LCD_ShowString (u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)
{
    u8 x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')) {                                            //�ж��ǲ��ǷǷ��ַ�!   
        if (x >= width) {
            x = x0;
            y += size;
        }

        if (y >= height) {
            break; //�˳�
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
                return;		//��������
            }
            if((y-y0)==size) {
                y=y0;
                x++;
				if(x>=lcddev.width) {
                    return;	//��������
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
                return;		//��������
            }
            if((y-y0)==size) {
                y=y0;
                x++;
				if(x>=lcddev.width) {
                    return;	//��������
                }
                break;
            } 
         }  
     }  
 }














