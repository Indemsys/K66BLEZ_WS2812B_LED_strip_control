#ifndef SUPERVISOR_TASK_H
  #define SUPERVISOR_TASK_H


// ������� ���������� ������� ��� ���������� watchdog � ��
#define WATCHDOG_TIMEOUT     250  // ������� ������� ������� ������ watchdog
#define WATCHDOG_WIN         50   // ������ ������� ������� ������ watchdog
#define SUPERVISOR_TIMEOUT   100  // ������ ������� � �� �� ��������� �������� ������ Task_supervisor ���������� watchdog


void Task_supervisor(unsigned int initial_data);

void Write_start_log_rec(void);

#endif // SUPERVISOR_TASK_H



