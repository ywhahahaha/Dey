#include "bsp_lcd.h"
#include "bmp.h"
#include "main.h"
#include "module_lcd_gui.h"
#include "typedef.h"
uint8_t  Exist_t[CONFIG_MAX_I03T] = {0};                                        //对应的T存在赋1
extern DEApp_t *pDEApp;
extern NvbNature_t     NvbNature;
//查找有多少个T在线
void FindI03T (void) {
    uint8_t i;
    for (i = 0; i < CONFIG_MAX_I03T; i++) {
        if ((pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr !=0) && 
            (pDEApp->device_config.i03t_nodes[i].base_para.mount == 0)) {
                Exist_t[i] = 1;
            } else {
                Exist_t[i] = 0;
            }
    }
}

void AnalogueData (void) {   
    AlarmCell_t cell_alarm = {0};
    I03T_Info_t *i03t_node    = NULL;
    
//    NvbNature.AlarmPageNature.AlarmPageNum = 1;
//    NvbNature.CellPageNature.CellPageNum = 1;
    FindI03T();
    pDEApp->device_config.i03m.cell_numbers = 0;    
    for (uint16_t i; i < CONFIG_MAX_I03T; i++) {
        if (Exist_t[i]) {
            i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr);
            pDEApp->device_config.i03t_nodes[i].sys_para.cell_number = rand()%300;
            i03t_node->discharge.current[0] = rand()%10000; 
            i03t_node->discharge.status = i%3;
            i03t_node->discharge.voltage = rand()%10000;
            i03t_node->discharge.soc = rand()%1000;
            i03t_node->discharge.soh = rand()%1000;
            i03t_node->discharge.temp = rand()%50;
            i03t_node->discharge.available_time= rand()%50000;
            i03t_node->alarm.alarm.bat_group_alarm1 = 0xffffffff;
            i03t_node->alarm.alarm.bat_group_alarm2 = 0xffffffff;
            i03t_node->alarm.alarm.bat_group_alarm3 = 0xffffffff;

            pDEApp->device_config.i03m.cell_numbers += pDEApp->device_config.i03t_nodes[i].sys_para.cell_number;
            for (uint16_t j = 0; j < pDEApp->device_config.i03t_nodes[i].sys_para.cell_number; j++) {
                cell_alarm.all = 0;
                cell_alarm.bit.low_volt = 1;
                cell_alarm.bit.over_res = 2;
                cell_alarm.bit.over_temp = 3;
                cell_alarm.bit.over_volt = 1;
                cell_alarm.bit.comm_err = 1;
                cell_alarm.bit.temp_sample = 1;
                cell_alarm.bit.volt_sample = 1;
                i03t_node->hist.cells[j].inter_res = rand()%10000;
                i03t_node->hist.cells[j].temperature  = rand()%1000;
                i03t_node->hist.cells[j].voltage  = rand()%10000;                
                if (j%2 == 0) {
                    i03t_node->alarm.alarm.cell_alarm[j] = cell_alarm.all;
                } else {
                    i03t_node->alarm.alarm.cell_alarm[j] = 0;
                }
            }     
        }
     }
 }

//公司logo显示
//void DeyLogoShow (void) {
//    LCD_Color_Fill(1,1,94,50,gImage_deyi);
//}

//显示状态栏
void StatusShow (void) {
    DATE_yymmddhhmmss_t now;
    uint8_t             min;
    uint8_t             hour;
    uint16_t            month;
    uint16_t            day;
    uint8_t             date[6];
    uint8_t             Time[6];
    uint16_t            xGap;
    uint16_t            yGap;
    uint16_t            y;
   
 //   DeyLogoShow();
    
//    BACK_COLOR  = LIGHTBLUE;   
//    POINT_COLOR = WHITE;
//    Gapx        = 10;
//    y           = 15;
//    Chinese_Show_one(Gapx,y,0,24,0);
//    Gapx        += 25;
//    Chinese_Show_one(Gapx,y,34,24,0);
//    Gapx        += 25;
//    Chinese_Show_one(Gapx,y,2,24,0);
//    Gapx        += 25;
//    Chinese_Show_one(Gapx,y,3,24,0);
//    Gapx        += 25;
//    Chinese_Show_one(Gapx,y,4,24,0);
//    Gapx        += 25;
//    Chinese_Show_one(Gapx,y,5,24,0);
//    Gapx        += 25;
//    Chinese_Show_one(Gapx,y,6,24,0);

    BACK_COLOR  = LIGHTBLUE;   
    POINT_COLOR = WHITE;
    xGap        = 10;
    yGap        = 15;
    Chinese_Show_one(xGap,yGap,0,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,1,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,2,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,3,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,4,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,5,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,6,34,0);
    
    yGap = 55;
    LCD_DrawLine(0,yGap,800,yGap);
    LCD_DrawLine(0,yGap+1,800,yGap+1);
    
    yGap = 15;
    Bsp_RtcGetTime(&now);
    min  = now.bits.min; 
    hour = now.bits.hour;
    month = now.bits.month;
    day = now.bits.day;
    sprintf(Time,"%02d:%02d",hour,min);
    sprintf(date,"%02d-%02d",month,day);
    LCD_ShowString(600,yGap,80,24,32,date);
    LCD_ShowString(700,yGap,80,24,32,Time);      
}



void CellDiagram (void) {
    uint16_t xGap;
    uint16_t yGap;
    uint8_t  Tname[3];
    I03T_Info_t *i03t_node = NULL;
    
    i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[NvbNature.CellPageNature.Tindex].base_para.i03t_addr);    
    LCD_Fill(15,75,785,460,DARKBLUE);
//    LCD_Fill(15,75,785,105,LIGHTBLUE);
    
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;
    xGap = 260;
    yGap = 85;
    sprintf(Tname,"T%d",NvbNature.CellPageNature.Tindex+1);
    LCD_ShowString(xGap,yGap,16,16,16,Tname);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,100,16,0); 
    xGap += 16;
    Chinese_Show_one(xGap,yGap,45,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,2,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,54,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,83,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,101,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'(',16,0);
    xGap += 8;
    Chinese_Show_one(xGap,yGap,104,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,105,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,106,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,107,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'T',16,0);
    xGap += 20;
    Chinese_Show_one(xGap,yGap,108,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,109,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,110,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,111,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,')',16,0);
    
    xGap  = 15; 
    yGap  = 105;
    LCD_DrawLine(xGap,yGap,785,yGap);
    LCD_DrawLine(xGap,yGap+1,785,yGap+1);
    
    BACK_COLOR  = DARKBLUE;   
//    LCD_Fill(15,107,785,137,LIGHTBLUE);
    xGap = 120;
    yGap = 117;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,26,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'V',16,0);
    xGap += 78;    
    Chinese_Show_one(xGap,yGap,32,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,33,16,0);
    xGap += 16;
    LCD_ShowString(xGap,yGap,8,20,16,"u");
    xGap += 8;
    Chinese_Show_one(xGap,yGap,86,16,0); 
    xGap += 78;
    Chinese_Show_one(xGap,yGap,34,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,35,16,0);
    xGap += 16;
    LCD_ShowString(xGap,yGap,8,20,16,"C");
    
    xGap = 580;
    Chinese_Show_one(xGap,yGap,21,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,22,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,52,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,53,16,0);
    

//    LCD_Fill(15,138,80,460,DARKGRAY);
//    LCD_Fill(81,138,785,460,LIGHTWHITE);
}

void CellInfo (void) {
    uint16_t    xGap;
    uint16_t    yGap;
    uint8_t     CellIDX[5];                                                            //电池页码显示拼接字符数组
    uint8_t     CellVol[6];                                                            //电池电压显示拼接字符数组
    uint8_t     CellRes[6];                                                            //电池内阻显示拼接字符数组
    uint8_t     CellTem[6];     
    float       Num;
    AlarmCell_t cell_alarm = {0};
    I03T_Info_t *i03t_node = NULL;
    uint16_t    CellNum;
    uint16_t    pageNum;
    uint16_t    CellIndexStart;                                                        //电池显示起始序号
    uint16_t    CellIndexEnd;  
    uint8_t     PageTable[8];   
    
    //清屏
    LCD_Fill(30,150,785,460,DARKBLUE);
    
    if (NvbNature.OldTNumber != NvbNature.CellPageNature.Tindex) {
        NvbNature.CellPageNature.CellPageNum = 0;
    }
    NvbNature.OldTNumber = NvbNature.CellPageNature.Tindex;
    
    CellNum   = pDEApp->device_config.i03t_nodes[NvbNature.CellPageNature.Tindex].sys_para.cell_number;
    i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[NvbNature.CellPageNature.Tindex].base_para.i03t_addr);
    if (CellNum%9) {
        pageNum   = CellNum/9+1;
    } else {
        pageNum   = CellNum/9;
    }
    if (NvbNature.CellPageNature.CellPageNum >= (pageNum)) {                                            //是否超过了这个电池组最大页面
        NvbNature.CellPageNature.CellPageNum = 0;
    }        
    CellIndexStart = (NvbNature.CellPageNature.CellPageNum) * 9;       
    CellIndexEnd   = CellIndexStart + 9;
    if (CellIndexEnd > CellNum) {
        CellIndexEnd = CellNum;
    }

    xGap = 30;
    yGap = 150;
    for (; CellIndexStart  < CellIndexEnd; CellIndexStart++) {          
        //序号
        BACK_COLOR  = DARKBLUE;   
        POINT_COLOR = WHITE;
        sprintf(CellIDX,"T%d-%03d",NvbNature.CellPageNature.Tindex+1,CellIndexStart+1);
        LCD_ShowString(xGap,yGap,48,20,16,CellIDX);
        //电压
        xGap = 128;
        BACK_COLOR  = DARKBLUE;
        Num   = (float)i03t_node->hist.cells[CellIndexStart].voltage/1000;
        sprintf(CellVol,"%.3lf",Num);
        LCD_ShowString(xGap,yGap,40,20,16,CellVol);
        //内阻
        xGap += 118;            
        sprintf(CellRes,"%d",i03t_node->hist.cells[CellIndexStart].inter_res);            
        LCD_ShowString(xGap,yGap,40,20,16,CellRes);
        //温度
        xGap += 110;
        Num   = (float)i03t_node->hist.cells[CellIndexStart].temperature /10;
        sprintf(CellTem,"%.1lf",Num);
        LCD_ShowString(xGap,yGap,40,20,16,CellTem);
        
        cell_alarm.all = i03t_node->alarm.alarm.cell_alarm[CellIndexStart];        
        if(cell_alarm.all == 0) {
            POINT_COLOR = GREEN;
            xGap = 416;            
            Chinese_Show_one(xGap,yGap,70,16,0);
            xGap += 16; 
            Chinese_Show_one(xGap,yGap,87,16,0);
            xGap += 16; 
            Chinese_Show_one(xGap,yGap,22,16,0);
        } else {
            POINT_COLOR = ORANGE;
            xGap += 60;
            if(cell_alarm.bit.over_volt) {
              Chinese_Show_one(xGap,yGap,55,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,26,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.over_volt,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }
            if(cell_alarm.bit.low_volt) {
              Chinese_Show_one(xGap,yGap,88,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,26,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.low_volt,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }            
            if(cell_alarm.bit.over_temp) {
              Chinese_Show_one(xGap,yGap,55,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,34,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.over_temp,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }
            if(cell_alarm.bit.over_res) {
              Chinese_Show_one(xGap,yGap,32,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,33,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.over_res,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            } 
            if(cell_alarm.bit.comm_err) {
              Chinese_Show_one(xGap,yGap,89,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,90,16,0);
              xGap += 16;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }
            if(cell_alarm.bit.volt_sample) {
              Chinese_Show_one(xGap,yGap,9,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,26,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,91,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,92,16,0);
              xGap += 16;                
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            } 
            if(cell_alarm.bit.temp_sample) {
              Chinese_Show_one(xGap,yGap,34,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,35,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,91,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,92,16,0);
              xGap += 16;                
            }              
        }
        
        yGap  += 30;
        xGap  = 30;           
    }
    xGap = 380;
    yGap = 430;
    BACK_COLOR  = DARKBLUE;
    POINT_COLOR = WHITE;
    //显示页码
//    LCD_Fill(xGap-15,yGap,xGap+35,yGap+16,DARKGRAY);    
    sprintf(PageTable,"%d/%d",NvbNature.CellPageNature.CellPageNum+1,pageNum);
    LCD_ShowString(xGap,yGap,48,20,16,PageTable);
    xGap = 30;
    yGap = 430;
    BACK_COLOR  = DARKBLUE;
    Chinese_Show_one(xGap,yGap,102,16,0);
    xGap = 760;
    yGap = 430;
    BACK_COLOR  = DARKBLUE;
    Chinese_Show_one(xGap,yGap,103,16,0);
        
}

//void CellAlarmInfo (void) {
//    uint16_t xGap;
//    uint16_t yGap;
//    
//    LCD_Fill(15,75,785,460,DARKBLUE);
//    LCD_Fill(15,75,785,105,BROWN);
//    
//    BACK_COLOR  = BROWN;   
//    POINT_COLOR = WHITE;
//    xGap = 350;
//    yGap = 85;
//    Chinese_Show_one(xGap,yGap,100,16,0); 
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,45,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,2,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,54,16,0);   
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,21,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,22,16,0);       
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,83,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,101,16,0);
//    
//    xGap  = 15; 
//    yGap  = 105;
//    LCD_DrawLine(xGap,yGap,785,yGap);
//    LCD_DrawLine(xGap,yGap+1,785,yGap+1);
//    
//    BACK_COLOR  = BLACK;   
//    LCD_Fill(15,107,785,137,BLACK);
//    xGap = 180;
//    yGap = 117;
//    Chinese_Show_one(xGap,yGap,21,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,22,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,52,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,53,16,0);
//    
//    
//    xGap = 590;
//    Chinese_Show_one(xGap,yGap,21,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,22,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,52,16,0);
//    xGap += 16;
//    Chinese_Show_one(xGap,yGap,53,16,0);

//    LCD_Fill(15,138,80,460,DARKGRAY);
//    LCD_Fill(81,138,410,460,LIGHTWHITE);
//    LCD_Fill(400,138,465,460,DARKGRAY);
//    LCD_Fill(466,138,785,460,LIGHTWHITE);
//    
//}

void MainMenuInfo (void) {
    uint16_t    xGap;
    uint16_t    yGap;
    uint16_t    TCellNum;
    uint16_t    NormalNum = 0;
    uint16_t    GenAlarmNum = 0;
    uint16_t    ImpAlarmNum = 0;
    uint16_t    SerAlarmNum = 0;
    float       Num;
    uint8_t     TName[3];
    uint8_t     VolNum[6];
    uint8_t     CurNum[6];
    uint8_t     SocNum[5];
    uint8_t     SohNum[5];
    uint8_t     ComNum[5];
    AlarmCell_t cell_alarm = {0};
    I03T_Info_t *i03t_node    = NULL;    

    //清掉数据显示的地方
    //设备信息
    xGap = 162;
    yGap = 200;
    LCD_Fill(xGap,yGap,xGap+8,yGap+16,DARKBLUE);
    xGap = 264;
    LCD_Fill(xGap,yGap,xGap+32,yGap+16,DARKBLUE);
    //单体告警信息
    xGap  = 50;
    yGap  = 345;    
    LCD_Fill(xGap,yGap,xGap+64,yGap+32,DARKBLUE);
    xGap += 160;
    LCD_Fill(xGap,yGap,xGap+64,yGap+32,DARKBLUE);
    xGap  = 50;
    yGap  = 415;
    LCD_Fill(xGap,yGap,xGap+64,yGap+32,DARKBLUE);
    xGap += 160;
    LCD_Fill(xGap,yGap,xGap+64,yGap+32,DARKBLUE);
    //电池组信息
    xGap = 340;
    yGap = 160;
    LCD_Fill(xGap,yGap,xGap+420,yGap+240,DARKBLUE);
    
    
    //设备信息
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;
    xGap = 162;
    yGap = 200;        
    TCellNum = pDEApp->device_config.i03m.i03t_number;
    LCD_ShowxNum(xGap,yGap,TCellNum,1,16,0);
    xGap = 264;
    memset(ComNum,0,sizeof(ComNum));
    TCellNum = pDEApp->device_config.i03m.cell_numbers;
    sprintf(ComNum,"%d",TCellNum);
    LCD_ShowString(xGap,yGap,48,20,16,ComNum);
    
    //单体告警信息
    FindI03T();    
    //统计单体正常和告警数量
    for (uint16_t i; i < CONFIG_MAX_I03T; i++) {
        if (Exist_t[i]) {
            i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr);
            for (uint16_t j = 0; j < pDEApp->device_config.i03t_nodes[i].sys_para.cell_number; j++) {
                cell_alarm.all = i03t_node->alarm.alarm.cell_alarm[j];
                if (cell_alarm.all == 0) {
                    NormalNum++;
                } else {
                    if (cell_alarm.bit.over_volt == 1) {
                        GenAlarmNum++;                        
                    } else if (cell_alarm.bit.over_volt == 2) {
                        ImpAlarmNum++;
                    } else if (cell_alarm.bit.over_volt == 3) {
                        SerAlarmNum++;
                    }
                    if (cell_alarm.bit.low_volt == 1) {
                        GenAlarmNum++;                        
                    } else if (cell_alarm.bit.low_volt == 2) {
                        ImpAlarmNum++;
                    } else if (cell_alarm.bit.low_volt == 3) {
                        SerAlarmNum++;
                    }                    
                    if (cell_alarm.bit.over_temp == 1) {
                        GenAlarmNum++;                        
                    } else if (cell_alarm.bit.over_temp == 2) {
                        ImpAlarmNum++;
                    } else if (cell_alarm.bit.over_temp == 3) {
                        SerAlarmNum++;
                    }
                    if (cell_alarm.bit.over_res == 1) {
                        GenAlarmNum++;                        
                    } else if (cell_alarm.bit.over_res == 2) {
                        ImpAlarmNum++;
                    } else if (cell_alarm.bit.over_res == 3) {
                        SerAlarmNum++;
                    }
                    if (cell_alarm.bit.volt_sample) {
                        SerAlarmNum++;                        
                    } 
                    if (cell_alarm.bit.temp_sample) {
                        SerAlarmNum++;                        
                    }                     
               }
            }
        } 
    }
    xGap  = 50;
    yGap  = 345;
    POINT_COLOR = GREEN;
    memset(ComNum,0,sizeof(ComNum));    
    sprintf(ComNum,"%d",NormalNum);
    LCD_ShowString(xGap,yGap,64,32,32,ComNum);
    POINT_COLOR = ACIDBLLUE;
    xGap += 160;
    memset(ComNum,0,sizeof(ComNum));    
    sprintf(ComNum,"%d",GenAlarmNum);
    LCD_ShowString(xGap,yGap,64,32,32,ComNum);
    xGap  = 50;
    yGap  = 415;
    POINT_COLOR = ORANGE;
    memset(ComNum,0,sizeof(ComNum));    
    sprintf(ComNum,"%d",ImpAlarmNum);
    LCD_ShowString(xGap,yGap,64,32,32,ComNum);
    POINT_COLOR = RED;
    xGap += 160;
    memset(ComNum,0,sizeof(ComNum));    
    sprintf(ComNum,"%d",SerAlarmNum);
    LCD_ShowString(xGap,yGap,64,32,32,ComNum);
   
    //电池组信息
    xGap = 340;
    yGap = 160;
    POINT_COLOR = WHITE;
    for(uint16_t i = 0; i < CONFIG_MAX_I03T; i++) {
        if (Exist_t[i]) {                                                           //判断这个T是否存在
            sprintf(TName,"T%d",i+1);
            LCD_ShowString(xGap,yGap,16,20,16,TName);
            
            xGap += 68;
            i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr);
                       
            //状态
            POINT_COLOR = GREEN;
            if(i03t_node->discharge.status == IDLE_STATE) {
                Chinese_Show_one(xGap,yGap,68,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,62,16,0);
            } else if (i03t_node->discharge.status == CHARGE_STATE) {
                Chinese_Show_one(xGap,yGap,7,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,9,16,0);                
            } else if (i03t_node->discharge.status == DISCHARGE_STATE) {
                Chinese_Show_one(xGap,yGap,8,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,9,16,0); 
            } else {
                Chinese_Show_one(xGap,yGap,12,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,7,16,0);                 
            }
            //电流
           POINT_COLOR = WHITE;
           xGap += 52;           
           Num = (float)i03t_node->discharge.current[0]/10;
           sprintf(CurNum,"%.1lf",Num);
           LCD_ShowString(xGap,yGap,48,20,16,CurNum);
           
            //电压
           xGap += 68;           
           Num = (float)i03t_node->discharge.voltage/10;
           sprintf(VolNum,"%.1lf",Num);
           LCD_ShowString(xGap,yGap,48,20,16,VolNum);
           
            //SOC
           xGap += 68;           
           Num = (float)i03t_node->discharge.soc/10;
           sprintf(SocNum,"%.1lf",Num);
           LCD_ShowString(xGap,yGap,48,20,16,SocNum);
            
            //SOH
           xGap += 60;           
           Num = (float)i03t_node->discharge.soh/10;
           sprintf(SohNum,"%.1lf",Num);
           LCD_ShowString(xGap,yGap,48,20,16,SohNum); 
            
            //报警
           xGap += 60;
           if((pDEApp->alarm[i].bat_group_alarm1 == 0) &&
              (pDEApp->alarm[i].bat_group_alarm2 == 0) &&
              (pDEApp->alarm[i].bat_group_alarm3 == 0) ) {
                POINT_COLOR = GREEN;
                Chinese_Show_one(xGap,yGap,70,16,0);
           } else {
                POINT_COLOR = RED;                
                Chinese_Show_one(xGap,yGap,69,16,0);
           }
         yGap    = yGap + 40;                                                             //下一行向下平移
         xGap = 340;
         POINT_COLOR = WHITE;
        }      
    }
}

void MainMenuDiagram (void) {
    uint16_t    xGap;
    uint16_t    yGap;
    //设备信息    
    LCD_Fill(15,75,300,260,DARKBLUE);
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;
    
    xGap = 20;
    yGap = 80;
    Chinese_Show_one(xGap,yGap,97,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,73,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,71,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,72,16,0);
    
    xGap  = 15; 
    yGap  = 105;
    LCD_DrawLine(xGap,yGap,300,yGap);
    LCD_DrawLine(xGap,yGap+1,300,yGap+1);
     
    xGap  = 300;
    yGap  = 75;
    LCD_DrawLine(xGap,yGap,xGap,260);
    LCD_DrawLine(xGap+1,yGap,xGap+1,260);
    
    xGap  = 15;
    yGap  = 260;
    LCD_DrawLine(xGap,yGap,300,yGap);
    LCD_DrawLine(xGap,yGap+1,300,yGap+1);

    POINT_COLOR = ACIDBLLUE;
    xGap = 20;
    yGap = 150;
    Image_Show(xGap,yGap,2,40,0);
    xGap += 50;
    Image_Show(xGap,yGap,1,40,0);
    xGap += 50;
    Image_Show(xGap,yGap,0,40,0);
    xGap += 50;
    Image_Show(xGap,yGap,1,40,0);
    xGap += 50;
    Image_Show(xGap,yGap,3,40,0);
    
    POINT_COLOR = WHITE;
    xGap = 23;
    yGap = 200;
    LCD_ShowString(xGap,yGap,64,16,16,"I03M");
    xGap += 75;
    LCD_ShowString(xGap,yGap,16,16,16,"T");
    xGap += 8;
    Chinese_Show_one(xGap,yGap,98,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,83,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,31,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,':',16,16);
    xGap += 8;
//    LCD_ShowString(xGap,yGap,8,16,16,"6");//填写T的个数
    //电池总数
    xGap += 30;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,14,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,93,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,83,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,':',16,16);
    xGap += 8;
//    LCD_ShowString(xGap,yGap,32,16,16,"1000");//填写电池总数
    //单体告警信息
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;
    LCD_Fill(15,270,300,460,DARKBLUE);
    xGap = 20;
    yGap = 280;
    Chinese_Show_one(xGap,yGap,2,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,54,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,87,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,22,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,71,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,72,16,0);
    
    xGap  = 15; 
    yGap  = 305;
    LCD_DrawLine(xGap,yGap,300,yGap);
    LCD_DrawLine(xGap,yGap+1,300,yGap+1);    
    
    xGap  = 300; 
    yGap  = 270;
    LCD_DrawLine(xGap,yGap,xGap,460);
    LCD_DrawLine(xGap+1,yGap+1,xGap+1,460);       
    
    xGap  = 15;
    yGap  = 460;
    LCD_DrawLine(xGap,yGap,300,yGap);
    LCD_DrawLine(xGap,yGap+1,300,yGap+1); 

    xGap  = 40;
    yGap  = 320;
    Chinese_Show_one(xGap,yGap,7,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,8,24,0);
    xGap  += 130;
    Chinese_Show_one(xGap,yGap,11,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,12,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,17,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,18,24,0);

    xGap  = 50;
    yGap  = 345;
    POINT_COLOR = GREEN;
//    LCD_ShowString(xGap,yGap,16,16,32,"0");//正常数量
    POINT_COLOR = ACIDBLLUE;
    xGap += 185;
//    LCD_ShowString(xGap,yGap,16,16,32,"0");//一般告警数量
    
    xGap  = 40;
    yGap  = 380;
    POINT_COLOR = WHITE;
    Chinese_Show_one(xGap,yGap,13,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,14,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,17,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,18,24,0);
    xGap  += 82;
    Chinese_Show_one(xGap,yGap,15,24,0);
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,16,24,0);   
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,17,24,0); 
    xGap  += 24;
    Chinese_Show_one(xGap,yGap,18,24,0);     
    
    xGap  = 50;
    yGap  = 415;
    POINT_COLOR = ORANGE;
//    LCD_ShowString(xGap,yGap,16,16,32,"0");//紧急告警数量
    POINT_COLOR = RED;
    xGap += 185;
//    LCD_ShowString(xGap,yGap,16,16,32,"0");//严重告警数量
    //电池组信息
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;
    LCD_Fill(310,75,785,460,DARKBLUE);
    xGap = 315;
    yGap = 80;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,14,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,16,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,71,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,72,16,0);
    
    xGap  = 310; 
    yGap  = 105;
    LCD_DrawLine(xGap,yGap,785,yGap);
    LCD_DrawLine(xGap,yGap+1,785,yGap+1);    
    
    xGap  = 310; 
    yGap  = 460;
    LCD_DrawLine(xGap,yGap,785,yGap);
    LCD_DrawLine(xGap,yGap+1,785,yGap+1);       
    
    xGap  = 785;
    yGap  = 75;
    LCD_DrawLine(xGap,yGap,xGap,460);
    LCD_DrawLine(xGap+1,yGap,xGap+1,460);

    
    xGap = 330;
    yGap = 120;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,14,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,16,16,0);
    xGap += 44;
    Chinese_Show_one(xGap,yGap,10,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,11,16,0);
    xGap += 44;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,27,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'A',16,0);
    xGap += 36;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,26,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'V',16,0); 
    xGap += 36;
    LCD_ShowString(xGap,yGap,32,20,16,"SOC%");
    xGap += 60;
    LCD_ShowString(xGap,yGap,32,20,16,"SOH%");
    xGap += 60;
    Chinese_Show_one(xGap,yGap,21,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,22,16,0);     
    
    //文字提示
    xGap = 410;
    yGap = 420;
    Chinese_Show_one(xGap,yGap,51,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,52,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,53,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,54,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,76,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,77,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,78,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,55,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,56,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,48,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,55,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,56,24,0);
        
    
}


//主菜单显示
void MainMenu (void) {   
    LCD_Clear(LIGHTBLUE);
    StatusShow();                                                                   //显示状态栏
    MainMenuDiagram();
    MainMenuInfo();
}

void BaryInfo (void) {
    uint16_t    xGap;
    uint16_t    yGap;
    float       Num;
    uint16_t    i;
    uint8_t     SpareTimeNum[8];
    uint8_t     SocNum[5];
    uint8_t     SohNum[5];
    uint8_t     CurNum[6];
    uint8_t     ComNum[6];
    uint8_t     Count = 0;
    I03T_Info_t *i03t_node = NULL;
    AlarmGroup1_t alarm_group1 = {0};
    AlarmGroup2_t alarm_group2 = {0};
    AlarmGroup3_t alarm_group3 = {0};
    
    i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[NvbNature.TPageNature.Tindex].base_para.i03t_addr);
    
    //填充
    //系统信息
    xGap = 110;
    yGap = 130;
    LCD_Fill(xGap,yGap,xGap+48,yGap+16,DARKBLUE);
    xGap += 118;
    LCD_Fill(xGap,yGap,xGap+48,yGap+16,DARKBLUE);
    xGap = 110;
    yGap = 185;
    LCD_Fill(xGap,yGap,xGap+48,yGap+16,DARKBLUE);
    xGap += 118;
    LCD_Fill(xGap,yGap,xGap+48,yGap+16,DARKBLUE);
    //状态信息
    xGap  = 410; 
    yGap  = 165;
    LCD_Fill(xGap,yGap,xGap+360,yGap+200,DARKBLUE);
    //告警信息
    xGap = 20;
    yGap = 280;
    LCD_Fill(xGap,yGap,xGap+278,yGap+178,DARKBLUE);
    
    
    //系统信息
    POINT_COLOR = WHITE;
    BACK_COLOR = DARKBLUE;
    xGap = 110;
    yGap = 130;
    LCD_ShowString(xGap,yGap,48,16,16,"0");                                 //湿度目前还没加  
    xGap += 118;
    memset(ComNum,0,sizeof(ComNum));
    Num = (float)i03t_node->discharge.temp/10;
    sprintf(ComNum,"%.1lf",Num);
    LCD_ShowString(xGap,yGap,48,20,16,ComNum);
    xGap = 110;
    yGap = 185;
    sprintf(ComNum,"%d",pDEApp->device_config.i03t_nodes[NvbNature.TPageNature.Tindex].sys_para.cell_number);
    LCD_ShowString(xGap,yGap,48,20,16,ComNum);
    xGap += 118;
    memset(ComNum,0,sizeof(ComNum));
    Num = (float)i03t_node->discharge.voltage/10;
    sprintf(ComNum,"%.1lf",Num);
    LCD_ShowString(xGap,yGap,48,20,16,ComNum);
    
    //状态信息
    xGap  = 410; 
    yGap  = 165;
    for (i = 0; i < 4; i++) {                    
        //状态       
        POINT_COLOR = GREEN;
        if(i03t_node->discharge.status == IDLE_STATE) {
            Chinese_Show_one(xGap,yGap,68,16,0);
            xGap += 16;
            Chinese_Show_one(xGap,yGap,62,16,0);
        } else if (i03t_node->discharge.status == CHARGE_STATE) {
            Chinese_Show_one(xGap,yGap,7,16,0);
            xGap += 16;
            Chinese_Show_one(xGap,yGap,9,16,0);                
        } else if (i03t_node->discharge.status == DISCHARGE_STATE) {
            Chinese_Show_one(xGap,yGap,8,16,0);
            xGap += 16;
            Chinese_Show_one(xGap,yGap,9,16,0); 
        } else {
            Chinese_Show_one(xGap,yGap,12,16,0);
            xGap += 16;
            Chinese_Show_one(xGap,yGap,7,16,0);                 
        }
       
       //电流
       POINT_COLOR = WHITE;
       xGap += 60;          
       Num = (float)i03t_node->discharge.current[i]/10;
       sprintf(CurNum,"%.1lf",Num);
       LCD_ShowString(xGap,yGap,48,20,16,CurNum);        

        //SOC
       xGap += 74;          
       Num = (float)i03t_node->discharge.soc/10;
       sprintf(SocNum,"%.1lf",Num);
       LCD_ShowString(xGap,yGap,48,20,16,SocNum);
        
        //SOH
       xGap += 74;           
       Num = (float)i03t_node->discharge.soh/10;
       sprintf(SohNum,"%.1lf",Num);
       LCD_ShowString(xGap,yGap,48,20,16,SohNum); 

        //备用时长
       xGap += 76;  
       Num = (float)i03t_node->discharge.available_time/10;       
       sprintf(SpareTimeNum,"%.1fh",Num);
       LCD_ShowString(xGap,yGap,64,20,16,SpareTimeNum);
       
       xGap = 410;
       yGap += 50;       
    }

    //告警信息

    alarm_group1.all = i03t_node->alarm.alarm.bat_group_alarm1;
    alarm_group2.all = i03t_node->alarm.alarm.bat_group_alarm2;
    alarm_group3.all = i03t_node->alarm.alarm.bat_group_alarm3; 
    
    if (!(alarm_group1.all||alarm_group2.all||alarm_group3.all)) {
        POINT_COLOR = GREEN;
        xGap = 100;
        yGap = 330;
        Chinese_Show_one(xGap,yGap,59,24,0);
        xGap += 24; 
        Chinese_Show_one(xGap,yGap,26,24,0);
        xGap += 24; 
        Chinese_Show_one(xGap,yGap,27,24,0);
        POINT_COLOR = WHITE;
    } else { 
       xGap = 20;
       yGap = 280;
       POINT_COLOR = ORANGE;
       if(alarm_group1.bit.over_volt) {
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,74,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,alarm_group1.bit.over_volt,1,24);
            xGap += 12;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;
            Count++;
       }
        if(alarm_group1.bit.low_volt) {
            Chinese_Show_one(xGap,yGap,75,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,74,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,alarm_group1.bit.low_volt,1,24);
            xGap += 12;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12; 
            Count++;
        } 
       if(alarm_group1.bit.over_temp) {
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,61,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,alarm_group1.bit.over_temp,1,24);
            xGap += 12;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;
            Count++;
        }
        if(alarm_group1.bit.low_soc) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }
            LCD_ShowString(xGap,yGap,36,24,24,"SOC");
            xGap += 36;
            Chinese_Show_one(xGap,yGap,62,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,alarm_group1.bit.low_soc,1,24);
            xGap += 12;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;           
        }
        if(alarm_group2.bit.total_volt_sample) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,63,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,74,24,0);
            xGap += 24;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;                        
        }        
        if(alarm_group2.bit.current1_sample) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,64,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,65,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,1,1,24);
            xGap += 12;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;                                 
        }        
        if(alarm_group2.bit.temp_sample) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,61,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,66,24,0);
            xGap += 24;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;                      
        } 
        if(alarm_group2.bit.comm_err) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,67,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,68,24,0);
            xGap += 24;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;                        
        }
        if(alarm_group2.bit.leak) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,69,24,0);
            xGap += 24; 
            Chinese_Show_one(xGap,yGap,70,24,0);
            xGap += 24;
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 12;                        
        }
        
        if(alarm_group3.bit.current1_over_charge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,1,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,72,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;               
        }
        if(alarm_group3.bit.current1_over_discharge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,1,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,73,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;                 
        }
        if(alarm_group3.bit.current2_over_charge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,2,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,72,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;                 
        }
        if(alarm_group3.bit.current2_over_discharge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,2,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,73,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;                 
        }
        if(alarm_group3.bit.current3_over_charge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,3,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,72,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;                
        }
        if(alarm_group3.bit.current3_over_discharge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,3,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,73,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;                
        }
        if(alarm_group3.bit.current4_over_charge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,4,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,72,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;                
        }
        if(alarm_group3.bit.current4_over_discharge) {
            if (Count++%3 == 0) {
                xGap = 20;
                yGap += 30;
            }            
            Chinese_Show_one(xGap,yGap,71,24,0);
            xGap += 24;
            LCD_ShowNum(xGap,yGap,4,1,24);
            xGap += 12;  
            Chinese_Show_one(xGap,yGap,60,24,0);
            xGap += 24;
            Chinese_Show_one(xGap,yGap,73,24,0);
            xGap += 24;  
            LCD_ShowChar(xGap,yGap,',',24,0);
            xGap += 8;              
        }
        
    }
      
}

void BaryDiagram (void) {
    uint16_t xGap;
    uint16_t yGap;
    uint8_t  Tname[3];
    I03T_Info_t *i03t_node = NULL;
    
    i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[NvbNature.TPageNature.Tindex].base_para.i03t_addr);
    //系统信息
    BACK_COLOR  = PURPLISHBULE;   
    POINT_COLOR = WHITE;
    LCD_Fill(15,75,300,460,DARKBLUE);
    LCD_Fill(15,75,300,105,BROWN);
    
    BACK_COLOR = BROWN;
    xGap = 20;
    yGap = 85;
    sprintf(Tname,"T%d",NvbNature.TPageNature.Tindex+1);
    LCD_ShowString(xGap,yGap,16,16,16,Tname);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,81,16,0); 
    xGap += 16;
    Chinese_Show_one(xGap,yGap,82,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,71,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,72,16,0);

    BACK_COLOR = DARKBLUE;
    xGap  = 15; 
    yGap  = 105;
    LCD_DrawLine(xGap,yGap,300,yGap);
    LCD_DrawLine(xGap,yGap+1,300,yGap+1);
    
    xGap = 30;
    yGap = 130;
    Chinese_Show_one(xGap,yGap,99,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,35,16,0);
    xGap += 16;
    LCD_ShowString(xGap,yGap,32,16,16,"%LH:");    

    
    xGap += 113;
    Chinese_Show_one(xGap,yGap,34,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,35,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'C',16,0);
    xGap += 8;
    LCD_ShowString(xGap,yGap,8,16,16,":");
    xGap = 30;
    yGap = 185;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,14,16,0);    
    xGap += 16;
    Chinese_Show_one(xGap,yGap,83,16,0); 
    xGap += 16;
    Chinese_Show_one(xGap,yGap,31,16,0); 
    xGap += 16;
    LCD_ShowString(xGap,yGap,8,16,16,":");
    xGap = 175;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,26,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'V',16,0);
    xGap += 8;
    LCD_ShowString(xGap,yGap,8,16,16,":");    

    //告警信息
    LCD_Fill(15,240,300,270,BROWN);
    BACK_COLOR = BROWN;
    POINT_COLOR = WHITE;
    xGap = 20;
    yGap = 250;
    sprintf(Tname,"T%d",NvbNature.TPageNature.Tindex+1);
    LCD_ShowString(xGap,yGap,16,16,16,Tname);
    xGap += 16;    
    Chinese_Show_one(xGap,yGap,87,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,22,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,71,16,0);    
    xGap += 16;
    Chinese_Show_one(xGap,yGap,72,16,0);
    
    xGap  = 15; 
    yGap  = 270;
    LCD_DrawLine(xGap,yGap,300,yGap);
    LCD_DrawLine(xGap,yGap+1,300,yGap+1);
    
    xGap  = 15; 
    yGap  = 460;
    LCD_DrawLine(xGap,yGap,300,yGap);
    LCD_DrawLine(xGap,yGap+1,300,yGap+1);

    xGap  = 300; 
    yGap  = 75;
    LCD_DrawLine(xGap,yGap,xGap,460);
    LCD_DrawLine(xGap+1,yGap,xGap+1,460);
    
    //状态信息
    LCD_Fill(320,75,785,460,DARKBLUE);
    LCD_Fill(320,75,785,105,BROWN);
    xGap = 330;
    yGap = 85;
    sprintf(Tname,"T%d",NvbNature.TPageNature.Tindex+1);
    LCD_ShowString(xGap,yGap,16,16,16,Tname);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,10,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,11,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,71,16,0);    
    xGap += 16;
    Chinese_Show_one(xGap,yGap,72,16,0);    

    xGap  = 320; 
    yGap  = 105;
    LCD_DrawLine(xGap,yGap,785,yGap);
    LCD_DrawLine(xGap,yGap+1,785,yGap+1);
    
    xGap  = 320; 
    yGap  = 460;
    LCD_DrawLine(xGap,yGap,785,yGap);
    LCD_DrawLine(xGap,yGap+1,785,yGap+1);

    xGap  = 785; 
    yGap  = 75;
    LCD_DrawLine(xGap,yGap,xGap,460);
    LCD_DrawLine(xGap+1,yGap,xGap+1,460);
    
    xGap  = 410; 
    yGap  = 130;
    BACK_COLOR = DARKBLUE;
    Chinese_Show_one(xGap,yGap,10,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,11,16,0);
    xGap  += 50;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,27,16,0);
    xGap  += 16;
    LCD_ShowChar(xGap,yGap,'A',16,0);
    xGap  += 42;
    LCD_ShowString(xGap,yGap,32,20,16,"SOC%");
    xGap  += 76;
    LCD_ShowString(xGap,yGap,32,20,16,"SOH%");
    xGap  += 76;
    Chinese_Show_one(xGap,yGap,73,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,74,16,0);
    xGap  += 16;    
    Chinese_Show_one(xGap,yGap,45,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,75,16,0);     

    xGap  = 340; 
    yGap  = 165;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,27,16,0);     
    xGap  += 16;
    LCD_ShowString(xGap,yGap,8,16,16,"1");
    xGap  = 340; 
    yGap  += 50;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,27,16,0);     
    xGap  += 16;
    LCD_ShowString(xGap,yGap,8,16,16,"2");
    xGap  = 340; 
    yGap  += 50;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,27,16,0);     
    xGap  += 16;
    LCD_ShowString(xGap,yGap,8,16,16,"3");
    xGap  = 340; 
    yGap  += 50;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap  += 16;
    Chinese_Show_one(xGap,yGap,27,16,0);     
    xGap  += 16;
    LCD_ShowString(xGap,yGap,8,16,16,"4");    

    //文字提示
    xGap  = 380; 
    yGap  = 400;
    Chinese_Show_one(xGap,yGap,51,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,52,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,79,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,80,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,34,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,87,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,85,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,86,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,81,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,82,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,83,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,84,24,0);
    xGap += 25; 
}

void ShowT (void) {
    LCD_Clear(LIGHTBLUE);
    StatusShow();                                                                   //显示状态栏    
    BaryDiagram();
    BaryInfo();
}

void ShowCell (void) {
    LCD_Clear(LIGHTBLUE);
    StatusShow();                                                                   //显示状态栏        
    CellDiagram();
    CellInfo();    
}

void NvgBarDiagram1 (void) {
    uint16_t xGap;
    uint16_t yGap;
    
    LCD_Fill(15,75,785,460,DARKBLUE);
    
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;   
    
    
    xGap = 300;
    yGap = 110;
    POINT_COLOR = GREEN;
    Image_Show(xGap,yGap,10,50,0);
    POINT_COLOR = WHITE;
    xGap += 65;
    yGap += 15;
    Chinese_Show_one(xGap,yGap,37,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,33,24,0);
    xGap += 35;     
    Chinese_Show_one(xGap,yGap,22,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,23,24,0);
    
    xGap = 300;
    yGap += 50;
    POINT_COLOR = GREEN;
    Image_Show(xGap,yGap,9,50,0);
    POINT_COLOR = WHITE;
    xGap += 65;
    yGap += 15;    
    Chinese_Show_one(xGap,yGap,1,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,35,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,36,24,0);    
    xGap += 35;
    Chinese_Show_one(xGap,yGap,22,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,23,24,0);    

    xGap = 300;
    yGap += 50;
    POINT_COLOR = GREEN;
    Image_Show(xGap,yGap,3,50,0);
    POINT_COLOR = WHITE;
    yGap += 15;
    xGap += 65;
    Chinese_Show_one(xGap,yGap,1,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,35,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,22,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,23,24,0);     
    
    
    xGap = 300;
    yGap += 50;
    POINT_COLOR = GREEN;
    Image_Show(xGap,yGap,7,50,0);
    POINT_COLOR = WHITE;
    yGap += 15;
    xGap += 65;
    Chinese_Show_one(xGap,yGap,1,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,35,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,26,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,27,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,22,24,0);
    xGap +=35;
    Chinese_Show_one(xGap,yGap,23,24,0);        
    
    xGap = 300;
    yGap += 50;
    POINT_COLOR = GREEN;
    Image_Show(xGap,yGap,6,50,0);
    POINT_COLOR = WHITE;
    yGap += 15;
    xGap += 60;
    Chinese_Show_one(xGap,yGap,28,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,29,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,30,24,0);
    xGap += 35;
    Chinese_Show_one(xGap,yGap,31,24,0);
    
}

void NvgBarDiagram (void) {
    DATE_yymmddhhmmss_t now;
    uint8_t             min;
    uint8_t             hour;    
    uint16_t            month;
    uint16_t            day;
    uint8_t             Time[6];
    uint8_t             date[6];
    uint16_t            xGap;
    uint16_t            yGap;
    uint16_t            xWidth;
    uint16_t            yHeith; 
    
    
    xGap = 0;
    yGap = 0;
    xWidth = 800;
    yHeith = 160;    
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,LIGHTBLACK);
    
    
    BACK_COLOR  = LIGHTBLACK;   
    POINT_COLOR = WHITE;
    xGap        = 10;
    yGap        = 40;
    Chinese_Show_one(xGap,yGap,0,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,1,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,2,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,3,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,4,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,5,34,0);
    xGap        += 35;
    Chinese_Show_one(xGap,yGap,6,34,0);
    
   
    Bsp_RtcGetTime(&now);
    
    min  = now.bits.min; 
    hour = now.bits.hour;    
    month = now.bits.month;
    day = now.bits.day;
    sprintf(Time,"%02d:%02d",hour,min);
    sprintf(date,"%02d-%02d",month,day);
    LCD_ShowString(600,yGap,80,24,32,date);
    LCD_ShowString(700,yGap,80,24,32,Time);     

    xGap = 0;
    yGap = 161;
    xWidth = 800;
    yHeith = 319;    
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,LIGHTGRAY);


    xGap = 15;
    yGap = 130;
    xWidth = 770;
    yHeith = 225;    
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,WHITE);

    xGap = 30;
    yGap = 160;
    xWidth = 125;
    yHeith = 175;
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,DARKPINK);
    xGap += 150;
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,BULEBRED);
    xGap += 150;
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,DARKGREEN);
    xGap += 150;
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,DARKGREEN);
    xGap += 150;
    LCD_Fill(xGap,yGap,xGap+xWidth ,yGap+yHeith,DARKRED);
    
    xGap = 95;
    yGap = 175;
    BACK_COLOR  = DARKPINK;   
    POINT_COLOR = WHITE;
    Image_Show(xGap,yGap,10,50,0);
    xGap += 150;
    BACK_COLOR  = BULEBRED; 
    Image_Show(xGap,yGap,9,50,0);
    xGap += 150;
    BACK_COLOR  = DARKGREEN;
    Image_Show(xGap,yGap,3,50,0);
    xGap += 150;
    BACK_COLOR  = DARKGREEN;
    Image_Show(xGap,yGap,7,50,0);
    xGap += 150;
    BACK_COLOR  = DARKRED;
    Image_Show(xGap,yGap,6,50,0);
    
    xGap = 37;
    yGap = 290;
    BACK_COLOR  = DARKPINK;   
    POINT_COLOR = WHITE;
    Chinese_Show_one(xGap,yGap,38,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,39,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,32,24,0); 
    xGap += 30;
    Chinese_Show_one(xGap,yGap,33,24,0);   
    xGap += 60;    
    BACK_COLOR  = BULEBRED;   
    Chinese_Show_one(xGap,yGap,42,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,43,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,44,24,0); 
    xGap += 30;
    Chinese_Show_one(xGap,yGap,45,24,0);
    xGap += 60;    
    BACK_COLOR  = DARKGREEN;   
    Chinese_Show_one(xGap,yGap,40,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,41,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,44,24,0); 
    xGap += 30;
    Chinese_Show_one(xGap,yGap,45,24,0);
    xGap += 60;    
    BACK_COLOR  = DARKGREEN;   
    Chinese_Show_one(xGap,yGap,26,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,27,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,22,24,0); 
    xGap += 30;
    Chinese_Show_one(xGap,yGap,23,24,0);    
    xGap += 60;    
    BACK_COLOR  = DARKRED;   
    Chinese_Show_one(xGap,yGap,28,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,29,24,0);
    xGap += 30;
    Chinese_Show_one(xGap,yGap,30,24,0); 
    xGap += 30;
    Chinese_Show_one(xGap,yGap,31,24,0);

    xGap = 170;
    yGap = 405;
    BACK_COLOR  = LIGHTGRAY;   
    POINT_COLOR = BLACK;
    Chinese_Show_one(xGap,yGap,46,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,47,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,48,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,49,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,50,24,0);
    xGap += 40;
    Chinese_Show_one(xGap,yGap,51,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,52,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,48,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,53,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,54,24,0);
    xGap += 40;
    Chinese_Show_one(xGap,yGap,55,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,56,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,48,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,55,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,56,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,57,24,0);
    xGap += 25;
    Chinese_Show_one(xGap,yGap,32,24,0);
    xGap += 25; 
    Chinese_Show_one(xGap,yGap,58,24,0);
       
}

void ShowNvgBar (void) {
    
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;    
    LCD_Fill(0,75,800,480,LIGHTBLUE);
    LCD_Fill(15,75,785,460,DARKBLUE);
//    NvgBarDiagram1();
    NvgBarDiagram();
    
}

void IniTNumber (void) {
    uint8_t i;
    FindI03T();
    for (i = 0; i < CONFIG_MAX_I03T; i++) {
        if (Exist_t[i]) {
            NvbNature.TPageNature.Tindex = i;
            NvbNature.CellPageNature.Tindex = i;
            NvbNature.AlarmPageNature.Tindex = i;
            break;
        }
    }
   
   if (i >= CONFIG_MAX_I03T) {
        NvbNature.TPageNature.Tindex = 0;
        NvbNature.CellPageNature.Tindex = 0;
        NvbNature.AlarmPageNature.Tindex = 0;    
   }
}

//void ShowCellAlarm (void) {
//    BACK_COLOR  = DARKBLUE;   
//    POINT_COLOR = WHITE;
//    LCD_Fill(15,75,786,461,LIGHTBLUE);
//    CellAlarmInfo();
//}


void AlarmDiagram (void) {
    uint16_t xGap;
    uint16_t yGap;
    uint8_t  Tname[3];
    I03T_Info_t *i03t_node = NULL;
    
    
    i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[NvbNature.AlarmPageNature.Tindex].base_para.i03t_addr);    
    LCD_Fill(15,75,785,460,DARKBLUE);
//    LCD_Fill(15,75,785,105,LIGHTBLUE);
    
    BACK_COLOR  = DARKBLUE;   
    POINT_COLOR = WHITE;
    xGap = 260;
    yGap = 85;
    sprintf(Tname,"T%d",NvbNature.AlarmPageNature.Tindex+1);
    LCD_ShowString(xGap,yGap,16,16,16,Tname);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,100,16,0); 
    xGap += 16;
    Chinese_Show_one(xGap,yGap,45,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,87,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,22,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,83,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,101,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'(',16,0);
    xGap += 8;
    Chinese_Show_one(xGap,yGap,104,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,105,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,106,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,107,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'T',16,0);
    xGap += 20;
    Chinese_Show_one(xGap,yGap,108,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,109,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,110,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,111,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,')',16,0);
    
    
    xGap  = 15; 
    yGap  = 105;
    LCD_DrawLine(xGap,yGap,785,yGap);
    LCD_DrawLine(xGap,yGap+1,785,yGap+1);
    
    BACK_COLOR  = DARKBLUE;   
//    LCD_Fill(15,107,785,137,LIGHTBLUE);
    xGap = 120;
    yGap = 117;
    Chinese_Show_one(xGap,yGap,9,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,26,16,0);
    xGap += 16;
    LCD_ShowChar(xGap,yGap,'V',16,0);
    xGap += 78;    
    Chinese_Show_one(xGap,yGap,32,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,33,16,0);
    xGap += 16;
    LCD_ShowString(xGap,yGap,8,20,16,"u");
    xGap += 8;
    Chinese_Show_one(xGap,yGap,86,16,0); 
    xGap += 78;
    Chinese_Show_one(xGap,yGap,34,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,35,16,0);
    xGap += 16;
    LCD_ShowString(xGap,yGap,8,20,16,"C");
    
    xGap = 580;
    Chinese_Show_one(xGap,yGap,21,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,22,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,52,16,0);
    xGap += 16;
    Chinese_Show_one(xGap,yGap,53,16,0);

    
//    LCD_Fill(15,138,80,460,DARKGRAY);
//    LCD_Fill(81,138,785,460,LIGHTWHITE);
}


uint16_t CellArlamCount (uint8_t Tindex) {
    I03T_Info_t    *i03t_node    = NULL;
    uint16_t        CellNum;
    AlarmCell_t     cell_alarm = {0};
    uint16_t        AlarmNUm = 0;
       
    CellNum   = pDEApp->device_config.i03t_nodes[Tindex].sys_para.cell_number;            
    i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[Tindex].base_para.i03t_addr);
    for (uint16_t j = 0; j < CellNum; j++) {
        cell_alarm.all = i03t_node->alarm.alarm.cell_alarm[j];
        if (cell_alarm.all) {
            AlarmNUm++;
        }
    }
        
   return AlarmNUm;
    
}


void AlarmInfo (void) {
    uint16_t    xGap;
    uint16_t    yGap;
    uint8_t     CellIDX[5];                                                            //电池页码显示拼接字符数组
    uint8_t     CellVol[6];                                                            //电池电压显示拼接字符数组
    uint8_t     CellRes[6];                                                            //电池内阻显示拼接字符数组
    uint8_t     CellTem[6];
    uint8_t     TIDX[5];     
    float       Num;
    AlarmCell_t cell_alarm = {0};
    I03T_Info_t *i03t_node = NULL;
    uint16_t    AlarmNum;
    uint16_t    pageNum;
    uint16_t    CellIndexStart;                                                        //电池显示起始序号
    uint16_t    CellIndexEnd;  
    uint8_t     PageTable[8];
    uint16_t    AlarmCount;
    uint8_t     Count = 0;
    uint16_t    CellNumber = 0;    
    AlarmGroup1_t alarm_group1 = {0};
    AlarmGroup2_t alarm_group2 = {0};
    AlarmGroup3_t alarm_group3 = {0};    
    
    
    //清屏
    LCD_Fill(15,150,785,465,DARKBLUE);
     
    if (NvbNature.OldTNumber != NvbNature.AlarmPageNature.Tindex) {
        NvbNature.AlarmPageNature.AlarmPageNum = 0;
    }
    NvbNature.OldTNumber = NvbNature.AlarmPageNature.Tindex;
    i03t_node = i03t_node_find(pDEApp->device_config.i03t_nodes[NvbNature.AlarmPageNature.Tindex].base_para.i03t_addr);
      
    AlarmNum  = CellArlamCount(NvbNature.AlarmPageNature.Tindex);
    
    if ((AlarmNum + 1)%9) {
        pageNum   = (AlarmNum + 1)/9+1;
    } else {
        pageNum   = (AlarmNum + 1)/9;
    }
    if (NvbNature.AlarmPageNature.AlarmPageNum >= (pageNum)) {                                            //是否超过了这个电池组最大页面
        NvbNature.AlarmPageNature.AlarmPageNum = 0;
    }
    if (NvbNature.AlarmPageNature.AlarmPageNum == 0) {
        CellIndexStart = 0;
    } else {
        CellIndexStart = (NvbNature.AlarmPageNature.AlarmPageNum) * 9 - 1;
    }

    AlarmCount = CellIndexStart + 1;    
    
    if (NvbNature.AlarmPageNature.AlarmPageNum == 0) {
        xGap = 30;
        yGap = 150;
        BACK_COLOR  = DARKBLUE;   
        POINT_COLOR = WHITE;
        sprintf(TIDX,"T%d-000",NvbNature.AlarmPageNature.Tindex+1);
        LCD_ShowString(xGap,yGap,48,20,16,TIDX);
        alarm_group1.all = i03t_node->alarm.alarm.bat_group_alarm1;
        alarm_group2.all = i03t_node->alarm.alarm.bat_group_alarm2;
        alarm_group3.all = i03t_node->alarm.alarm.bat_group_alarm3;        
        if (!(alarm_group1.all||alarm_group2.all||alarm_group3.all)) {
            POINT_COLOR = WHITE;
            BACK_COLOR  = DARKBLUE; 
            xGap = 430;        
            Chinese_Show_one(xGap,yGap,70,16,0);
            xGap += 16; 
            Chinese_Show_one(xGap,yGap,87,16,0);
            xGap += 16; 
            Chinese_Show_one(xGap,yGap,22,16,0);        
        } else { 
           xGap = 100;       
           POINT_COLOR = ORANGE;
           BACK_COLOR  = DARKBLUE;
           if(alarm_group1.bit.over_volt) {
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16; 
                Chinese_Show_one(xGap,yGap,26,16,0);
                xGap += 16; 
                LCD_ShowNum(xGap,yGap,alarm_group1.bit.over_volt,1,16);
                xGap += 8;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;            
           }
            if(alarm_group1.bit.low_volt) {
                Chinese_Show_one(xGap,yGap,88,16,0);
                xGap += 16;  
                Chinese_Show_one(xGap,yGap,26,16,0);
                xGap += 16; 
                LCD_ShowNum(xGap,yGap,alarm_group1.bit.low_volt,1,16);
                xGap += 8;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;             
            } 
           if(alarm_group1.bit.over_temp) {
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;  
                Chinese_Show_one(xGap,yGap,34,16,0);
                xGap += 16; 
                LCD_ShowNum(xGap,yGap,alarm_group1.bit.over_temp,1,16);
                xGap += 8;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;           
            }
            if(alarm_group1.bit.low_soc) { 
                LCD_ShowString(xGap,yGap,24,16,16,"SOC");
                xGap += 24;
                Chinese_Show_one(xGap,yGap,63,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,alarm_group1.bit.low_soc,1,16);
                xGap += 8;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;           
            }
            if(alarm_group2.bit.total_volt_sample) {       
                Chinese_Show_one(xGap,yGap,93,16,0);
                xGap += 16; 
                Chinese_Show_one(xGap,yGap,26,16,0);
                xGap += 16;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                        
            }        
            if(alarm_group2.bit.current1_sample) {          
                Chinese_Show_one(xGap,yGap,9,16,0);
                xGap += 16; 
                Chinese_Show_one(xGap,yGap,27,16,0);
                xGap += 16; 
                LCD_ShowNum(xGap,yGap,1,1,16);
                xGap += 8;  
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                                 
            }        
            if(alarm_group2.bit.temp_sample) {          
                Chinese_Show_one(xGap,yGap,34,16,0);
                xGap += 16; 
                Chinese_Show_one(xGap,yGap,35,16,0);
                xGap += 16;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                      
            } 
            if(alarm_group2.bit.comm_err) {          
                Chinese_Show_one(xGap,yGap,89,16,0);
                xGap += 16; 
                Chinese_Show_one(xGap,yGap,90,16,0);
                xGap += 16;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                        
            }
            if(alarm_group2.bit.leak) {         
                Chinese_Show_one(xGap,yGap,94,16,0);
                xGap += 16; 
                Chinese_Show_one(xGap,yGap,95,16,0);
                xGap += 16;
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                        
            }
            
            if(alarm_group3.bit.current1_over_charge) {          
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,1,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,96,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',24,0);
                xGap += 8;               
            }
            if(alarm_group3.bit.current1_over_discharge) {         
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,1,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,51,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                 
            }
            if(alarm_group3.bit.current2_over_charge) {          
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,2,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,96,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',24,0);
                xGap += 8;               
            }
            if(alarm_group3.bit.current2_over_discharge) {         
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,2,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,51,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                 
            }
            if(alarm_group3.bit.current3_over_charge) {          
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,3,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,96,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',24,0);
                xGap += 8;               
            }
            if(alarm_group3.bit.current3_over_discharge) {         
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,3,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,51,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                 
            }
            if(alarm_group3.bit.current4_over_charge) {          
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,4,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,96,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',24,0);
                xGap += 8;               
            }
            if(alarm_group3.bit.current4_over_discharge) {         
                Chinese_Show_one(xGap,yGap,16,16,0);
                xGap += 16;
                LCD_ShowNum(xGap,yGap,4,1,16);
                xGap += 8;  
                Chinese_Show_one(xGap,yGap,55,16,0);
                xGap += 16;
                Chinese_Show_one(xGap,yGap,51,16,0);
                xGap += 16;  
                LCD_ShowChar(xGap,yGap,',',16,0);
                xGap += 8;                 
            }
        }
    }  

    for (CellNumber = 0; CellNumber < pDEApp->device_config.i03t_nodes[NvbNature.AlarmPageNature.Tindex].sys_para.cell_number; CellNumber++) {
        if (i03t_node->alarm.alarm.cell_alarm[CellNumber]) {
            if ((--AlarmCount) == 0) {                                   
                break;
            }
        }
    }
    xGap = 30;
    if (NvbNature.AlarmPageNature.AlarmPageNum == 0) {
        yGap = 180;
    } else {
        yGap = 150;
    }
    
    for (;  CellNumber < pDEApp->device_config.i03t_nodes[NvbNature.AlarmPageNature.Tindex].sys_para.cell_number; CellNumber++) {          
        cell_alarm.all = i03t_node->alarm.alarm.cell_alarm[CellNumber];
        if(cell_alarm.all) {
            //序号
            BACK_COLOR  = DARKBLUE;   
            POINT_COLOR = WHITE;
            sprintf(CellIDX,"T%d-%03d",NvbNature.CellPageNature.Tindex+1,CellNumber+1);
            LCD_ShowString(xGap,yGap,48,20,16,CellIDX);
            //电压
            xGap = 128;
            BACK_COLOR  = DARKBLUE;
            Num   = (float)i03t_node->hist.cells[CellNumber].voltage/1000;
            sprintf(CellVol,"%.3lf",Num);
            LCD_ShowString(xGap,yGap,40,20,16,CellVol);
            //内阻
            xGap += 118;            
            sprintf(CellRes,"%d",i03t_node->hist.cells[CellNumber].inter_res);            
            LCD_ShowString(xGap,yGap,40,20,16,CellRes);
            //温度
            xGap += 110;
            Num   = (float)i03t_node->hist.cells[CellNumber].temperature /10;
            sprintf(CellTem,"%.1lf",Num);
            LCD_ShowString(xGap,yGap,40,20,16,CellTem);
                                    
            POINT_COLOR = ORANGE;
            xGap += 60;
            if(cell_alarm.bit.over_volt) {
              Chinese_Show_one(xGap,yGap,55,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,26,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.over_volt,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }
            if(cell_alarm.bit.low_volt) {
              Chinese_Show_one(xGap,yGap,88,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,26,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.low_volt,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }            
            if(cell_alarm.bit.over_temp) {
              Chinese_Show_one(xGap,yGap,55,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,34,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.over_temp,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }
            if(cell_alarm.bit.over_res) {
              Chinese_Show_one(xGap,yGap,32,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,33,16,0);
              xGap += 16;
              LCD_ShowNum(xGap,yGap,cell_alarm.bit.over_res,1,16);
              xGap += 8;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            } 
            if(cell_alarm.bit.comm_err) {
              Chinese_Show_one(xGap,yGap,89,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,90,16,0);
              xGap += 16;
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            }
            if(cell_alarm.bit.volt_sample) {
              Chinese_Show_one(xGap,yGap,9,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,26,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,91,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,92,16,0);
              xGap += 16;                
              LCD_ShowChar(xGap,yGap,',',16,0);
              xGap += 8;
            } 
            if(cell_alarm.bit.temp_sample) {
              Chinese_Show_one(xGap,yGap,34,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,35,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,91,16,0);
              xGap += 16;
              Chinese_Show_one(xGap,yGap,92,16,0);
              xGap += 16;                
            }
            if(NvbNature.AlarmPageNature.AlarmPageNum == 0) {
                if(++Count == 8) { 
                    break;
                }
            }else {
                if(++Count == 9) { 
                    break;
                }
            }
            
            yGap  += 30;
            xGap  = 30; 
        }    
          
    }
    xGap = 380;
    yGap = 430;
    BACK_COLOR  = DARKBLUE;
    POINT_COLOR = WHITE;
    //显示页码
//    LCD_Fill(xGap-15,yGap,xGap+35,yGap+16,DARKGRAY);    
    sprintf(PageTable,"%d/%d",NvbNature.AlarmPageNature.AlarmPageNum+1,pageNum);
    LCD_ShowString(xGap,yGap,48,20,16,PageTable);
    xGap = 30;
    yGap = 430;
    BACK_COLOR  = DARKBLUE;
    Chinese_Show_one(xGap,yGap,102,16,0);
    xGap = 760;
    yGap = 430;
    BACK_COLOR  = DARKBLUE;
    Chinese_Show_one(xGap,yGap,103,16,0);
        
}

void ShowAlarm (void) {
    LCD_Clear(LIGHTBLUE);
    StatusShow();                                                                   //显示状态栏        
    AlarmDiagram();
    AlarmInfo();     
}

