#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "i03t_list.h"

static I03TNodeList_t i03node_list_head;

osSemaphoreId_t i03t_node_lock = NULL;

void i03t_node_init(void) {
    
    i03t_node_lock = osSemaphoreNew(1,1,NULL);
    
    INIT_LIST_HEAD(&i03node_list_head.list);
}

void i03t_node_load(void) {
    
    i03t_node_init();

    for(uint8_t i=0;i<CONFIG_MAX_I03T;i++) {
        
        i03t_node_add(0);
        
        if(pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr != 0 && pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR) {
            
            i03t_node_add_addr(pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr,
                               pDEApp->device_config.i03t_nodes[i].base_para.mount);
            
            debug_printf("Find I03T@%d,mount@%d\r\n",pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr,pDEApp->device_config.i03t_nodes[i].base_para.mount);
        }
    }
    
    i03t_node_update();
}

void i03t_node_update(void) {
    uint8_t temp = 0;
    for(uint8_t i=0;i<CONFIG_MAX_I03T;i++) {
        if(pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr != 0 && pDEApp->device_config.i03t_nodes[i].base_para.i03t_addr <= CONFIG_MAX_IO3T_MODBUS_ADDR) {
            temp++;
        }
    }
    
    pDEApp->device_config.i03m.i03t_number = temp;
}

void i03t_node_update_hist(uint16_t addr,CellHistStorage_t *hist) {
    
    osSemaphoreAcquire(i03t_node_lock,osWaitForever);
    struct list_head *pos, *n;
    I03TNodeList_t *p;
    list_for_each_safe(pos,n, &i03node_list_head.list) {
        p = list_entry(pos,I03TNodeList_t,list);
        if(p != NULL && p->node.i03t_addr == addr) {
            memcpy(p->node.hist.cells,hist->cells,sizeof(hist->cells));
			break;
        } 
    }
    
    osSemaphoreRelease(i03t_node_lock);
}

void i03t_node_update_alarm(uint16_t addr,AlarmStorage_t *alarm) {
    
    osSemaphoreAcquire(i03t_node_lock,osWaitForever);
    struct list_head *pos, *n;
    I03TNodeList_t *p;
    list_for_each_safe(pos,n, &i03node_list_head.list) {
        p = list_entry(pos,I03TNodeList_t,list);
        if(p != NULL && p->node.i03t_addr == addr) {
            //memcpy(p->node.alarm,alarm->cell_hist,sizeof(hist->cell_hist));
			break;
        } 
    }
    
    osSemaphoreRelease(i03t_node_lock);
}

I03T_Info_t *i03t_node_find(uint16_t addr) {

    struct list_head *pos, *n;
    I03TNodeList_t *p;
    
    list_for_each_safe(pos,n, &i03node_list_head.list) {
        p = list_entry(pos,I03TNodeList_t,list);
        if(p != NULL && p->node.i03t_addr == addr) {
			return &p->node;
        }
		
		if(pos == NULL || n == NULL) {
			break;
		}
    }
    return NULL;
}

bool i03t_node_add(uint16_t addr) {
    
    osSemaphoreAcquire(i03t_node_lock,osWaitForever);
    
    bool result = false;
    
    I03TNodeList_t *node = NULL;
    
    if(addr != 0) {
        if(i03t_node_find(addr) != NULL) {
            goto EXIT;
        }
    }


    node = (I03TNodeList_t *)sys_malloc(sizeof(I03TNodeList_t));
    if(node == NULL) goto EXIT;
   
    memset(node,0,sizeof(I03TNodeList_t));
    node->node.i03t_addr = addr;
    node->node.alarm.index = 0;
    node->node.hist.index = 0;
    node->node.flag.all = 0;
    node->node.discharge.soc = 1000;
    node->node.discharge.soh = 1000;
    node->node.discharge.deep_discharge_cycle = CONFIG_DEEP_DISCHARGE_CYCLE;

    list_add_tail(&node->list,&i03node_list_head.list);

EXIT:   
    osSemaphoreRelease(i03t_node_lock);
    
    return result;
}



bool i03t_node_add_addr(uint16_t addr,uint8_t mount) {
    osSemaphoreAcquire(i03t_node_lock,osWaitForever);
    
    bool result = false;

    if(i03t_node_find(addr) != NULL) {
        goto EXIT;
    }

    struct list_head *pos, *n;
    I03TNodeList_t *p;
    list_for_each_safe(pos,n, &i03node_list_head.list) {
        p = list_entry(pos,I03TNodeList_t,list);
        if(p != NULL && p->node.i03t_addr == 0) {
            memset(&p->node,0,sizeof(I03T_Info_t));
            p->node.i03t_addr = addr;
            p->node.mount = mount;
            p->node.discharge.soc = 1000;
            p->node.discharge.soh = 1000;
            p->node.discharge.deep_discharge_cycle = CONFIG_DEEP_DISCHARGE_CYCLE;
            p->node.discharge.index = 0;
            p->node.charge.index = 0;
            p->node.alarm.index = 0;
            p->node.hist.index = 0;
            p->node.flag.all = 0;
            
            result = true;
			break;
        } 
    }
    
EXIT:   
    osSemaphoreRelease(i03t_node_lock);
    
    return result;
}

bool i03t_node_remove_from_list(uint16_t addr) {
    osSemaphoreAcquire(i03t_node_lock,osWaitForever);
    
    bool result = false;
    struct list_head *pos, *n;
    I03TNodeList_t *p;
    list_for_each_safe(pos,n, &i03node_list_head.list) {
        p = list_entry(pos,I03TNodeList_t,list);
        if(p != NULL && p->node.i03t_addr == addr) {
            list_del(&p->list);
            sys_free(p);
            p = NULL;
			result = true;
        } 
    }
    
    osSemaphoreRelease(i03t_node_lock);
    
    return result;
}

void i03t_node_remove_addr(uint16_t addr) {
    osSemaphoreAcquire(i03t_node_lock,osWaitForever);
    
    struct list_head *pos, *n;
    I03TNodeList_t *p;
    list_for_each_safe(pos,n, &i03node_list_head.list) {
        p = list_entry(pos,I03TNodeList_t,list);
        if(p != NULL && p->node.i03t_addr == addr) {
            memset(&p->node,0,sizeof(I03T_Info_t));
            p->node.flag.all = 0;
            p->node.discharge.soc = 1000;
            p->node.discharge.soh = 1000;
            p->node.discharge.deep_discharge_cycle = CONFIG_DEEP_DISCHARGE_CYCLE;
        } 
    }

    osSemaphoreRelease(i03t_node_lock);

}



