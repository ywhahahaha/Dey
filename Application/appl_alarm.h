#ifndef __APPL_ALARM_H_
#define __APPL_ALARM_H_

void appl_alarm_check_init(void);
AlarmLevel_t appl_alarm_get_status(I03T_Info_t *i03t,AlarmType_t type);

#endif

