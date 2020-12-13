#include "msp430common.h"
#include "uart0.h"
#include "uart1.h"
#include "common.h"
#include "rom.h"
#include "flash.h"
#include "rtc.h"
#include "store.h"
#include "Sampler.h"
#include "flow.h"
#include "debug.h"
//#include "hydrology.h"
//#include "GPRS.h"
#include "message.h"
#include "hydrologycommand.h"

int FlowCheckSampleData(int* pstart,int* pend)
{
    //检查数据存储上标和 下标 
    int _startIdx=*pstart;
    int _endIdx=*pend;
    
    if(Hydrology_ReadStartIdx(&_startIdx)<0 || Hydrology_ReadEndIdx(&_endIdx)<0 )
    {//如果读出数据索引标记失败
        TraceMsg("read idx error ,reGenerate .",1);
        
        if(Hydrology_RetrieveIndex()<0)                                        //读出索引标记失败，则要生成索引标记++++++++++++++++++++++
        {
            TraceMsg("reGen failed .",1);
            Hydrology_SetStartIdx(HYDROLOGY_DATA_MIN_IDX);
            Hydrology_SetEndIdx(HYDROLOGY_DATA_MIN_IDX);
        }
        TraceMsg("StartIdx:",0); 
        TraceInt4(_startIdx,1);
        TraceMsg("EndIdx:",0);
        TraceInt4(_endIdx,1);
        Hydrology_ReadStartIdx(&_startIdx);//重新读出
        Hydrology_ReadEndIdx(&_endIdx);//重新读出
    }
    //下标正确性
    if( _endIdx<HYDROLOGY_DATA_MIN_IDX || _endIdx >HYDROLOGY_DATA_MAX_IDX || _startIdx<HYDROLOGY_DATA_MIN_IDX || _startIdx >HYDROLOGY_DATA_MAX_IDX)
    {
        TraceMsg("Idx bad .",1);
        return -1;
    }
    TraceMsg("startIdx=",1);
    TraceInt4(_startIdx,1);
    TraceMsg("endIdx=",1);
    TraceInt4(_endIdx,1);

      *pstart = _startIdx;
    *pend = _endIdx;
    
    return 0;
}

int FlowProcess()    //未使用  
{
    //检查数据存储上标和下标 
    int _startIdx=0;
    int _endIdx=0;
    char _effect_endIdx=0;//ky,前面的_endIdx的地址内的内容是已经读过了的，这里再定义一个地址，是保存有效数据的最后一个地址
    char _send[120] = {0}; 
    char _specSend[200] = {0};
    int  _ret=0;   
    int  _seek_num=0;//防止死循环
    int sendlen = 0;
   
    _ret = FlowCheckSampleData(&_startIdx,&_endIdx);           //获得startidx endidx
    if (_ret !=0 )
    {
        return -1;   
    }
    
    if(_endIdx == 1)
    {
        _effect_endIdx = 20;
    }
    else
    {
        _effect_endIdx = _endIdx-1;
    }

    while(1)
    {
        TraceMsg("read data in :",0);
        TraceInt4(_startIdx,1);
        if(_seek_num > DATA_ITEM_LEN)//寻找的数据条数已经超过最大值就退出     //+++要改为地址边界判断
        {
            TraceMsg("seek num out of range",1);
			//hydrologHEXfree();
                        System_Delayms(2000);
			System_Reset();
            //break;
        }

        //数据
        _ret = Store_ReadDataItem(_startIdx,_send,0);                           //+++++读取数据+++，startIDx++++ret为读出的数据长度
        if(_ret<0)
        {
            TraceMsg("can't read data ! very bad .",1);
            return -1; //无法读取数据 就直接退了.
        }
        else if(_ret==1)
        {//这个是一个已经发送过的数据,                                          //+++++++++++
            TraceMsg("It's sended data",1);
          //  if(_startIdx == _endIdx)
           // {//检查是否到了  _endIdx, 如果是就不继续循环了. 
               // _ret = hydrologHEXProcess(_send,42,TimerReport,_specSend,0);
				
               // break;
           // }
            //继续下一个位置
            if(_startIdx >= HYDROLOGY_DATA_MAX_IDX)                     //+++++如果读取的startidx超过可存的最大index，则重新置零
                _startIdx=HYDROLOGY_DATA_MIN_IDX;
            else   
                ++ _startIdx;//下一数据

            ++_seek_num;
			
                                                              //发送完后,要更新_startIdx.          
			Hydrology_SetStartIdx(_startIdx);
			
			TraceInt4(_startIdx, 1);
			TraceInt4(_endIdx, 1);
			
			hydrologHEXfree();

            //continue;
        }
        //是正常的发送数据,就增加_idx
        //增加下一次的 _startIdx   

	else //未发送的数据 //每条数据都要先向客户端服务器发三遍，在发送给本地服务器 
		{
	        sendlen = _ret;

	        if (_startIdx == _effect_endIdx)   //发送失败就跳出循环    
	        {
				Store_MarkDataItemSended(_startIdx);      
				_ret = hydrologHEXProcess(_send,_ret,TimerReport,_specSend,0);//发送最后一条数据，先向客户端发，再向本地发++++++++++++++++

				if(_ret !=0)
				{
					break;
				}
	        }
			
	        else
	        { 
				_ret = hydrologHEXProcess(_send,_ret,TimerReport,_specSend,1);//发送一条数据，向客户端发+++++++++++++
				
				if(_ret !=0)
	            {
	                break;
	            }	
	        }

	        //并设置该数据已发送
	        Store_MarkDataItemSended(_startIdx);

	        if(_startIdx>=HYDROLOGY_DATA_MAX_IDX) 
	            _startIdx=HYDROLOGY_DATA_MIN_IDX;
	        else   
	            ++ _startIdx;//下一数据

	        ++_seek_num;

	        TraceMsg(_send, 1);

	        //发送完后,要更新_startIdx. 
	        Hydrology_SetStartIdx(_startIdx);

	        TraceInt4(_startIdx, 1);
	        TraceInt4(_endIdx, 1);

	        hydrologHEXfree();

		}
    }

    TraceMsg("Report done",1);

    return 0;
}


