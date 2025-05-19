#ifndef ERROR_H
#define ERROR_H

// Флаг ошибок: 0 — всё OK, != 0 — обнаружена ошибка
extern int error_flag;

#define SET_ERROR()   (error_flag = 1)
#define CLEAR_ERROR() (error_flag = 0)
#define HAS_ERROR()   (error_flag != 0)

#endif // ERROR_H
