/*
 * author   :   howard
 * date     :   2019/09/29
 * Desc     :   这是一个用于输入输出调试信息的抽象层，只要是用于输入输出调试信息的模块设备，都需要按照本抽象层实例化对象。
 *              底层驱动需要向ioDev注册，上层便可使用IODev的操作函数。驱动注册详情参考blueTooth.c的691行-最后一行
*/

#ifndef __IO_MANAGER_H
#define __IO_MANAGER_H

typedef struct IODev
{
    char *name;
    int isCanUseFlag;   
    int (*isCanUse)(void);  //1 canUse, 0 can't use
    int (*init)(void);  // 0 success
    int (*open)(void);  // 0 success
    void (*getMsg)(char *msgRecv,int *len);
    int (*sendMsg)(char *msgSend,int len);  // 0 success
    int (*close)(void); // 0 success

    void (*restart)(void);
    int (*isinit)(void);
    int (*isspp)(void);
    void (*adv)(void)
}T_IODev,*PT_IODev;


/******Interfaces for up class **************************/

/*!
 * @brief : an IO device driver call this functin to register into Dev NodeList
 *
 * @param[in] ptIODev  : Structure instance of a specific device.
 *
 * @retval zero -> Success / other value -> Error.
 */
int RegisterIODev(PT_IODev ptIODev);

/*!
 * @brief : show IO device available
 */
void ShowIODevs(void);

/*!
 * @brief : get a IO device 
 *
 * @retval NULL -> appointed device is not registed / other value -> address of device's handler.
 */
PT_IODev getIODev(void);


#endif