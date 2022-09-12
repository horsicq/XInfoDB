/* Copyright (c) 2020-2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xinfodb.h"


bool _symbolSort(const XInfoDB::SYMBOL &v1,const XInfoDB::SYMBOL &v2)
{
    bool bResult=false;

    if(v1.nModule!=v2.nModule)
    {
        bResult=(v1.nModule<v2.nModule);
    }
    else
    {
        bResult=(v1.nAddress<v2.nAddress);
    }

    return bResult;
}

XInfoDB::XInfoDB(QObject *pParent) : QObject(pParent)
{
    g_mode=MODE_UNKNOWN;
#ifdef USE_XPROCESS
    g_processInfo={};
    g_handle=0;
#endif
    g_pDevice=nullptr;
    g_fileType=XBinary::FT_UNKNOWN;
    g_nMainModuleAddress=0;
    g_nMainModuleSize=0;

    XBinary::DM disasmMode=XBinary::DM_UNKNOWN;

#ifdef USE_XPROCESS
#ifdef Q_PROCESSOR_X86_32
    disasmMode=XBinary::DM_X86_32;
#endif
#ifdef Q_PROCESSOR_X86_64
    disasmMode=XBinary::DM_X86_64;
#endif
    XCapstone::openHandle(disasmMode,&g_handle,true);
#endif
}

XInfoDB::~XInfoDB()
{
#ifdef USE_XPROCESS
    XCapstone::closeHandle(&g_handle);
#endif
}

void XInfoDB::setDevice(QIODevice *pDevice,XBinary::FT fileType)
{
    g_pDevice=pDevice;
    g_fileType=fileType;
    g_mode=MODE_DEVICE;

    if(fileType==XBinary::FT_UNKNOWN)
    {
        g_fileType=XBinary::getPrefFileType(pDevice);
    }

    g_MainModuleMemoryMap=XFormats::getMemoryMap(g_fileType,pDevice);

    g_nMainModuleAddress=g_MainModuleMemoryMap.nModuleAddress;
    g_nMainModuleSize=g_MainModuleMemoryMap.nImageSize;
    g_sMainModuleName=XBinary::getDeviceFileBaseName(pDevice);
}

QIODevice *XInfoDB::getDevice()
{
    return g_pDevice;
}

XBinary::FT XInfoDB::getFileType()
{
    return g_fileType;
}

void XInfoDB::reload(bool bDataReload)
{
    emit dataChanged(bDataReload);
}

quint32 XInfoDB::read_uint32(XADDR nAddress,bool bIsBigEndian)
{
    quint32 nResult=0;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    nResult=XProcess::read_uint32(g_processInfo.hProcess,nAddress,bIsBigEndian);
#endif
#ifdef Q_OS_LINUX
    nResult=XProcess::read_uint32(g_processInfo.hProcessMemoryIO,nAddress,bIsBigEndian);
#endif
#endif
    // TODO XBinary
    return nResult;
}

quint64 XInfoDB::read_uint64(XADDR nAddress,bool bIsBigEndian)
{
    quint64 nResult=0;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    nResult=XProcess::read_uint64(g_processInfo.hProcess,nAddress,bIsBigEndian);
#endif
#ifdef Q_OS_LINUX
    nResult=XProcess::read_uint64(g_processInfo.hProcessMemoryIO,nAddress,bIsBigEndian);
#endif
#endif
    return nResult;
}

qint64 XInfoDB::read_array(XADDR nAddress,char *pData,quint64 nSize)
{
    qint64 nResult=0;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    nResult=XProcess::read_array(g_processInfo.hProcess,nAddress,pData,nSize);
#endif
#ifdef Q_OS_LINUX
    nResult=XProcess::read_array(g_processInfo.hProcessMemoryIO,nAddress,pData,nSize);
#endif
#endif
    return nResult;
}

qint64 XInfoDB::write_array(XADDR nAddress,char *pData,quint64 nSize)
{
    qint64 nResult=0;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    nResult=XProcess::write_array(g_processInfo.hProcess,nAddress,pData,nSize);
#endif
#ifdef Q_OS_LINUX
    nResult=XProcess::write_array(g_processInfo.hProcessMemoryIO,nAddress,pData,nSize);
#endif
#endif
    return nResult;
}

QByteArray XInfoDB::read_array(XADDR nAddress,quint64 nSize)
{
    QByteArray baResult;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    baResult=XProcess::read_array(g_processInfo.hProcess,nAddress,nSize);
#endif
#ifdef Q_OS_LINUX
    baResult=XProcess::read_array(g_processInfo.hProcessMemoryIO,nAddress,nSize);
#endif
#endif
    return baResult;
}

QString XInfoDB::read_ansiString(XADDR nAddress,quint64 nMaxSize)
{
    QString sResult;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    sResult=XProcess::read_ansiString(g_processInfo.hProcess,nAddress,nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult=XProcess::read_ansiString(g_processInfo.hProcessMemoryIO,nAddress,nMaxSize);
#endif
#endif
    return sResult;
}

QString XInfoDB::read_unicodeString(XADDR nAddress,quint64 nMaxSize)
{
    QString sResult;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    sResult=XProcess::read_unicodeString(g_processInfo.hProcess,nAddress,nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult=XProcess::read_unicodeString(g_processInfo.hProcessMemoryIO,nAddress,nMaxSize);
#endif
#endif
    return sResult;
}

QString XInfoDB::read_utf8String(XADDR nAddress,quint64 nMaxSize)
{
    QString sResult;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    sResult=XProcess::read_utf8String(g_processInfo.hProcess,nAddress,nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult=XProcess::read_utf8String(g_processInfo.hProcessMemoryIO,nAddress,nMaxSize);
#endif
#endif
    return sResult;
}
#ifdef USE_XPROCESS
bool XInfoDB::stepOverByHandle(X_HANDLE hThread,BPI bpInfo,bool bAddThreadBP)
{
    bool bResult=false;

    XADDR nAddress=getCurrentInstructionPointerByHandle(hThread);
    XADDR nNextAddress=getAddressNextInstructionAfterCall(nAddress);

    if(nNextAddress!=(XADDR)-1)
    {
        bResult=addBreakPoint(nNextAddress,XInfoDB::BPT_CODE_SOFTWARE,bpInfo,1);
    }
    else
    {
        if(bAddThreadBP)
        {
            XInfoDB::BREAKPOINT breakPoint={};
            breakPoint.bpType=XInfoDB::BPT_CODE_HARDWARE;
            breakPoint.bpInfo=bpInfo;

        #ifdef Q_OS_WIN
            getThreadBreakpoints()->insert(hThread,breakPoint);
        #endif
        }

        bResult=_setStepByHandle(hThread);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::stepOverById(X_ID nThreadId,BPI bpInfo,bool bAddThreadBP)
{
    bool bResult=false;

    XADDR nAddress=getCurrentInstructionPointerById(nThreadId);
    XADDR nNextAddress=getAddressNextInstructionAfterCall(nAddress);

    if(nNextAddress!=(XADDR)-1)
    {
        bResult=addBreakPoint(nNextAddress,XInfoDB::BPT_CODE_SOFTWARE,bpInfo,1);
    }
    else
    {
        if(bAddThreadBP)
        {
            XInfoDB::BREAKPOINT breakPoint={};
            breakPoint.bpType=XInfoDB::BPT_CODE_HARDWARE;
            breakPoint.bpInfo=bpInfo;

        #ifdef Q_OS_LINUX
            getThreadBreakpoints()->insert(nThreadId,breakPoint);
        #endif
        }

        bResult=_setStepById(nThreadId);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByAddress(XADDR nAddress,BPT bpType)
{
    BREAKPOINT result={};
    result.nAddress=-1;

    qint32 nNumberOfRecords=g_listBreakpoints.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if((g_listBreakpoints.at(i).nAddress==nAddress)&&(g_listBreakpoints.at(i).bpType==bpType))
        {
            result=g_listBreakpoints.at(i);

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByExceptionAddress(XADDR nExceptionAddress,BPT bpType)
{
    BREAKPOINT result={};
    result.nAddress=-1;

    qint32 nNumberOfRecords=g_listBreakpoints.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        XInfoDB::BREAKPOINT breakPoint=g_listBreakpoints.at(i);

        if(breakPoint.nAddress==(nExceptionAddress-breakPoint.nOrigDataSize))
        {
            result=breakPoint;

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::breakpointToggle(XADDR nAddress)
{
    bool bResult=false;

    if(!isBreakPointPresent(nAddress))
    {
        if(addBreakPoint(nAddress))
        {
            bResult=true;
        }
    }
    else
    {
        if(removeBreakPoint(nAddress))
        {
            bResult=true;
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::addSharedObjectInfo(SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    g_mapSharedObjectInfos.insert(pSharedObjectInfo->nImageBase,*pSharedObjectInfo);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::removeSharedObjectInfo(SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    g_mapSharedObjectInfos.remove(pSharedObjectInfo->nImageBase);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::addThreadInfo(THREAD_INFO *pThreadInfo)
{
    g_listThreadInfos.append(*pThreadInfo);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::removeThreadInfo(X_ID nThreadID)
{
    qint32 nNumberOfThread=g_listThreadInfos.count();

    for(qint32 i=0;i<nNumberOfThread;i++)
    {
        if(g_listThreadInfos.at(i).nThreadID==nThreadID)
        {
            g_listThreadInfos.removeAt(i);

            break;
        }
    }
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setFunctionHook(QString sFunctionName)
{
    bool bResult=false;

    qint64 nFunctionAddress=getFunctionAddress(sFunctionName);

    if(nFunctionAddress!=-1)
    {
        bResult=addBreakPoint(nFunctionAddress,XInfoDB::BPT_CODE_SOFTWARE,XInfoDB::BPI_FUNCTIONENTER,-1,sFunctionName);

        XInfoDB::FUNCTIONHOOK_INFO functionhook_info={};
        functionhook_info.sName=sFunctionName;
        functionhook_info.nAddress=nFunctionAddress;

        g_mapFunctionHookInfos.insert(sFunctionName,functionhook_info);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::removeFunctionHook(QString sFunctionName)
{
    bool bResult=false;
    // TODO Check !!!

    qint32 nNumberOfRecords=g_listBreakpoints.count();

    // TODO Check!
    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        XInfoDB::BREAKPOINT breakPoint=g_listBreakpoints.at(i);

        if(breakPoint.sInfo==sFunctionName)
        {
            g_listBreakpoints.removeAt(i);
        }
    }

    if(g_mapFunctionHookInfos.contains(sFunctionName))
    {
        g_mapFunctionHookInfos.remove(sFunctionName);

        bResult=true;
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
QMap<XADDR,XInfoDB::SHAREDOBJECT_INFO> *XInfoDB::getSharedObjectInfos()
{
    return &g_mapSharedObjectInfos;
}
#endif
#ifdef USE_XPROCESS
QList<XInfoDB::THREAD_INFO> *XInfoDB::getThreadInfos()
{
    return &g_listThreadInfos;
}
#endif
#ifdef USE_XPROCESS
QMap<QString,XInfoDB::FUNCTIONHOOK_INFO> *XInfoDB::getFunctionHookInfos()
{
    return &g_mapFunctionHookInfos;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::SHAREDOBJECT_INFO XInfoDB::findSharedInfoByName(QString sName)
{
    XInfoDB::SHAREDOBJECT_INFO result={};

    for(QMap<XADDR,XInfoDB::SHAREDOBJECT_INFO>::iterator it=g_mapSharedObjectInfos.begin();it!=g_mapSharedObjectInfos.end();)
    {
        if(it.value().sName==sName)
        {
            result=it.value();

            break;
        }

        ++it;
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::SHAREDOBJECT_INFO XInfoDB::findSharedInfoByAddress(XADDR nAddress)
{
    XInfoDB::SHAREDOBJECT_INFO result={};

    for(QMap<XADDR,XInfoDB::SHAREDOBJECT_INFO>::iterator it=g_mapSharedObjectInfos.begin();it!=g_mapSharedObjectInfos.end();)
    {
        XInfoDB::SHAREDOBJECT_INFO record=it.value();

        if((record.nImageBase<=nAddress)&&(record.nImageBase+record.nImageSize>nAddress))
        {
            result=record;

            break;
        }

        ++it;
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::THREAD_INFO XInfoDB::findThreadInfoByID(X_ID nThreadID)
{
    XInfoDB::THREAD_INFO result={};

    qint32 nNumberOfRecords=g_listThreadInfos.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(g_listThreadInfos.at(i).nThreadID==nThreadID)
        {
            result=g_listThreadInfos.at(i);

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
XInfoDB::THREAD_INFO XInfoDB::findThreadInfoByHandle(X_HANDLE hThread)
{
    XInfoDB::THREAD_INFO result={};

    qint32 nNumberOfRecords=g_listThreadInfos.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(g_listThreadInfos.at(i).hThread==hThread)
        {
            result=g_listThreadInfos.at(i);

            break;
        }
    }

    return result;
}
#endif
#endif
#ifdef USE_XPROCESS
quint64 XInfoDB::getFunctionAddress(QString sFunctionName)
{
    Q_UNUSED(sFunctionName)
    // TODO
    return 0;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setSingleStep(X_HANDLE hThread,QString sInfo)
{
    XInfoDB::BREAKPOINT breakPoint={};
    breakPoint.bpType=XInfoDB::BPT_CODE_HARDWARE;
    breakPoint.bpInfo=XInfoDB::BPI_STEPINTO;
    breakPoint.sInfo=sInfo;
#ifdef Q_OS_WIN
    getThreadBreakpoints()->insert(hThread,breakPoint);
#endif
    return _setStepByHandle(hThread);
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getAddressNextInstructionAfterCall(XADDR nAddress)
{
    XADDR nResult=-1;

    QByteArray baData=read_array(nAddress,15);

    XCapstone::OPCODE_ID opcodeID=XCapstone::getOpcodeID(g_handle,nAddress,baData.data(),baData.size());

    if(XCapstone::isCallOpcode(opcodeID.nOpcodeID))
    {
        nResult=nAddress+opcodeID.nSize;
    }

    return nResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::stepIntoByHandle(X_HANDLE hThread,BPI bpInfo,bool bAddThreadBP)
{
    if(bAddThreadBP)
    {
        XInfoDB::BREAKPOINT breakPoint={};
        breakPoint.bpType=XInfoDB::BPT_CODE_HARDWARE;
        breakPoint.bpInfo=bpInfo;
    #ifdef Q_OS_WIN
        getThreadBreakpoints()->insert(hThread,breakPoint);
    #endif
    }

    return _setStepByHandle(hThread);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::stepIntoById(X_ID nThreadId,BPI bpInfo,bool bAddThreadBP)
{
    if(bAddThreadBP)
    {
        XInfoDB::BREAKPOINT breakPoint={};
        breakPoint.bpType=XInfoDB::BPT_CODE_HARDWARE;
        breakPoint.bpInfo=bpInfo;
    #ifdef Q_OS_LINUX
        getThreadBreakpoints()->insert(nThreadId,breakPoint);
    #endif
    }

    return _setStepById(nThreadId);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::_setStepByHandle(X_HANDLE hThread)
{
    bool bResult=false;

    if(hThread)
    {
    #ifdef Q_OS_WIN
        CONTEXT context={0};
        context.ContextFlags=CONTEXT_CONTROL; // EFLAGS

        if(GetThreadContext(hThread,&context))
        {
            if(!(context.EFlags&0x100))
            {
                context.EFlags|=0x100;
            }

            if(SetThreadContext(hThread,&context))
            {
                bResult=true;
            }
        }
    #endif
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::_setStepById(X_ID nThreadId)
{
    bool bResult=false;
#ifdef Q_OS_LINUX
    errno=0;

    long int nRet=ptrace(PTRACE_SINGLESTEP,nThreadId,0,0);

    if(nRet==0)
    {
        bResult=true;
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("ptrace failed: %s",strerror(errno));
        // TODO error signal
    #endif
    }
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::suspendThreadById(X_ID nThreadId)
{
    bool bResult=false;

#ifdef Q_OS_LINUX
    if(syscall(SYS_tgkill,g_processInfo.nProcessID,nThreadId,SIGSTOP)!=-1)
    {
        // TODO Set thread status
        bResult=true;
    }
    else
    {
        qDebug("Cannot stop thread");
    }
#endif

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::suspendThreadByHandle(X_HANDLE hThread)
{
    bool bResult=false;
#ifdef Q_OS_WIN
    bResult=(SuspendThread(hThread)!=((DWORD)-1));
#endif
#ifdef QT_DEBUG
//    qDebug("XInfoDB::suspendThread %X",hThread);
#endif

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::resumeThreadByHandle(X_HANDLE hThread)
{
    bool bResult=false;
#ifdef Q_OS_WIN
    bResult=(ResumeThread(hThread)!=((DWORD)-1));
#endif
#ifdef QT_DEBUG
//    qDebug("XInfoDB::resumeThread %X",hThread);
#endif

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::suspendOtherThreads(X_ID nThreadId)
{
    bool bResult=false;

    QList<XInfoDB::THREAD_INFO> *pListThreads=getThreadInfos();

    qint32 nCount=pListThreads->count();

    // Suspend all other threads
    for(qint32 i=0;i<nCount;i++)
    {
        if(nThreadId!=pListThreads->at(i).nThreadID)
        {
        #ifdef Q_OS_WIN
            suspendThreadByHandle(pListThreads->at(i).hThread);
        #endif
            bResult=true;
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::resumeOtherThreads(X_ID nThreadId)
{
    bool bResult=false;

    QList<XInfoDB::THREAD_INFO> *pListThreads=getThreadInfos();

    qint32 nCount=pListThreads->count();

    for(qint32 i=0;i<nCount;i++)
    {
        if(nThreadId!=pListThreads->at(i).nThreadID)
        {
        #ifdef Q_OS_WIN
            resumeThreadByHandle(pListThreads->at(i).hThread);
        #endif
            bResult=true;
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::suspendAllThreads()
{
    bool bResult=false;

    QList<XInfoDB::THREAD_INFO> *pListThreads=getThreadInfos();

    qint32 nCount=pListThreads->count();

    // TODO Check if already suspended
    for(qint32 i=0;i<nCount;i++)
    {
    #ifdef Q_OS_WIN
        suspendThreadByHandle(pListThreads->at(i).hThread); // TODO Handle errors
    #endif
    #ifdef Q_OS_LINUX
        if(syscall(SYS_tgkill,g_processInfo.nProcessID,pListThreads->at(i).nThreadID,SIGSTOP)!=-1)
        {
//            int thread_status=0;

//            if(waitpid(pListThreads->at(i).nThreadID,&thread_status,__WALL)!=-1)
//            {
//                // TODO
//            }
        }
        else
        {
            qDebug("Cannot stop thread");
        }
    #endif
        bResult=true;
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::resumeAllThreads()
{
    bool bResult=false;

    QList<XInfoDB::THREAD_INFO> *pListThreads=getThreadInfos();

    qint32 nCount=pListThreads->count();

    // Resume all other threads
    for(qint32 i=0;i<nCount;i++)
    {
    #ifdef Q_OS_WIN
        resumeThreadByHandle(pListThreads->at(i).hThread);
    #endif
    #ifdef Q_OS_LINUX
        // TODO
        ptrace(PTRACE_CONT,pListThreads->at(i).nThreadID,0,0);
    #endif

        bResult=true;
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::FUNCTION_INFO XInfoDB::getFunctionInfo(X_HANDLE hThread,QString sName)
{
    FUNCTION_INFO result={};

#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_FULL; // Full

    if(GetThreadContext(hThread,&context))
    {
    #ifdef Q_PROCESSOR_X86_32
        quint64 nSP=(quint32)(context.Esp);
        quint64 nIP=(quint32)(context.Eip);
    #endif
    #ifdef Q_PROCESSOR_X86_64
        quint64 nSP=(quint64)(context.Rsp);
        quint64 nIP=(quint64)(context.Rip);
    #endif

        // TODO 64!
        result.nAddress=nIP;
        result.nRetAddress=read_uint32((quint32)nSP);
        result.nParameter0=read_uint32((quint32)(nSP+4+0*4));
        result.nParameter1=read_uint32((quint32)(nSP+4+1*4));
        result.nParameter2=read_uint32((quint32)(nSP+4+2*4));
        result.nParameter3=read_uint32((quint32)(nSP+4+3*4));
        result.nParameter4=read_uint32((quint32)(nSP+4+4*4));
        result.nParameter5=read_uint32((quint32)(nSP+4+5*4));
        result.nParameter6=read_uint32((quint32)(nSP+4+6*4));
        result.nParameter7=read_uint32((quint32)(nSP+4+7*4));
        result.nParameter8=read_uint32((quint32)(nSP+4+8*4));
        result.nParameter9=read_uint32((quint32)(nSP+4+9*4));
        result.sName=sName;
    }
#endif

    return result;
}
#endif
//#ifdef USE_XPROCESS
//bool XInfoDB::setStep(XProcess::HANDLEID handleThread)
//{
//    bool bResult=true;
//#if defined(Q_OS_LINUX)
//    if(ptrace(PTRACE_SINGLESTEP,handleThread.nID,0,0))
//    {
//        bResult=true;
////        int wait_status;
////        waitpid(g_hidThread.nID,&wait_status,0);
//    }
//#endif

//    return bResult;
//}
//#endif
//#ifdef USE_XPROCESS
//bool XInfoDB::stepInto(XProcess::HANDLEID handleThread)
//{
//    XInfoDB::BREAKPOINT breakPoint={};
//    breakPoint.bpType=XInfoDB::BPT_CODE_HARDWARE;
//    breakPoint.bpInfo=XInfoDB::BPI_STEPINTO;

//    g_mapThreadBreakpoints.insert(handleThread.nID,breakPoint);

//    return setStep(handleThread);
//}
//#endif
//#ifdef USE_XPROCESS
//bool XInfoDB::resumeThread(XProcess::HANDLEID handleThread)
//{
//    bool bResult=false;

//#if defined(Q_OS_LINUX)
//    if(ptrace(PTRACE_CONT,handleThread.nID,0,0))
//    {
//        bResult=true;
////        int wait_status;
////        waitpid(handleThread.nID,&wait_status,0);
//    }
//#endif

//    return bResult;
//}
//#endif
#ifdef USE_XPROCESS
void XInfoDB::setProcessInfo(PROCESS_INFO processInfo)
{
    g_processInfo=processInfo;
    g_mode=MODE_PROCESS;

    g_nMainModuleAddress=processInfo.nImageBase;
    g_nMainModuleSize=processInfo.nImageSize;
    g_sMainModuleName=g_processInfo.sBaseFileName;
    //g_MainModuleMemoryMap=XFormats::getMemoryMap(XBinary::FT_REGION,0,true,) // TODO getRegionMemoryMap
}
#endif
#ifdef USE_XPROCESS
XInfoDB::PROCESS_INFO *XInfoDB::getProcessInfo()
{
    return &g_processInfo;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateRegsById(X_ID nThreadId,XREG_OPTIONS regOptions)
{
    g_statusPrev.mapRegs=g_statusCurrent.mapRegs; // TODO save nThreadID

    g_statusCurrent.mapRegs.clear();
    g_statusCurrent.nThreadId=nThreadId;

#ifdef Q_OS_LINUX
    user_regs_struct regs={};
//    user_regs_struct regs;
    errno=0;

    if(ptrace(PTRACE_GETREGS,nThreadId,nullptr,&regs)!=-1)
    {
        if(regOptions.bGeneral)
        {
            g_statusCurrent.mapRegs.insert(XREG_RAX,XBinary::getXVariant((quint64)(regs.rax)));
            g_statusCurrent.mapRegs.insert(XREG_RBX,XBinary::getXVariant((quint64)(regs.rbx)));
            g_statusCurrent.mapRegs.insert(XREG_RCX,XBinary::getXVariant((quint64)(regs.rcx)));
            g_statusCurrent.mapRegs.insert(XREG_RDX,XBinary::getXVariant((quint64)(regs.rdx)));
            g_statusCurrent.mapRegs.insert(XREG_RBP,XBinary::getXVariant((quint64)(regs.rbp)));
            g_statusCurrent.mapRegs.insert(XREG_RSP,XBinary::getXVariant((quint64)(regs.rsp)));
            g_statusCurrent.mapRegs.insert(XREG_RSI,XBinary::getXVariant((quint64)(regs.rsi)));
            g_statusCurrent.mapRegs.insert(XREG_RDI,XBinary::getXVariant((quint64)(regs.rdi)));
            g_statusCurrent.mapRegs.insert(XREG_R8,XBinary::getXVariant((quint64)(regs.r8)));
            g_statusCurrent.mapRegs.insert(XREG_R9,XBinary::getXVariant((quint64)(regs.r9)));
            g_statusCurrent.mapRegs.insert(XREG_R10,XBinary::getXVariant((quint64)(regs.r10)));
            g_statusCurrent.mapRegs.insert(XREG_R11,XBinary::getXVariant((quint64)(regs.r11)));
            g_statusCurrent.mapRegs.insert(XREG_R12,XBinary::getXVariant((quint64)(regs.r12)));
            g_statusCurrent.mapRegs.insert(XREG_R13,XBinary::getXVariant((quint64)(regs.r13)));
            g_statusCurrent.mapRegs.insert(XREG_R14,XBinary::getXVariant((quint64)(regs.r14)));
            g_statusCurrent.mapRegs.insert(XREG_R15,XBinary::getXVariant((quint64)(regs.r15)));
        }

        if(regOptions.bIP)
        {
            g_statusCurrent.mapRegs.insert(XREG_RIP,XBinary::getXVariant((quint64)(regs.rip)));
        }

        if(regOptions.bFlags)
        {
            g_statusCurrent.mapRegs.insert(XREG_RFLAGS,XBinary::getXVariant((quint32)(regs.eflags)));
        }

        if(regOptions.bSegments)
        {
            g_statusCurrent.mapRegs.insert(XREG_GS,XBinary::getXVariant((quint16)(regs.gs)));
            g_statusCurrent.mapRegs.insert(XREG_FS,XBinary::getXVariant((quint16)(regs.fs)));
            g_statusCurrent.mapRegs.insert(XREG_ES,XBinary::getXVariant((quint16)(regs.es)));
            g_statusCurrent.mapRegs.insert(XREG_DS,XBinary::getXVariant((quint16)(regs.ds)));
            g_statusCurrent.mapRegs.insert(XREG_CS,XBinary::getXVariant((quint16)(regs.cs)));
            g_statusCurrent.mapRegs.insert(XREG_SS,XBinary::getXVariant((quint16)(regs.ss)));
        }
    }
    else
    {
        qDebug("errno: %s",strerror(errno));
    }

//    __extension__ unsigned long long int orig_rax;
//    __extension__ unsigned long long int fs_base;
//    __extension__ unsigned long long int gs_base;
#endif
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateRegsByHandle(X_HANDLE hThread,XREG_OPTIONS regOptions)
{
    g_statusPrev.mapRegs=g_statusCurrent.mapRegs; // TODO save nThreadID

    g_statusCurrent.mapRegs.clear();
    g_statusCurrent.hThread=hThread;

#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_ALL; // All registers TODO Check regOptions |CONTEXT_FLOATING_POINT|CONTEXT_EXTENDED_REGISTERS

    if(GetThreadContext(hThread,&context))
    {
        if(regOptions.bGeneral)
        {
        #ifdef Q_PROCESSOR_X86_32
            g_statusCurrent.mapRegs.insert(XREG_EAX,XBinary::getXVariant((quint32)(context.Eax)));
            g_statusCurrent.mapRegs.insert(XREG_EBX,XBinary::getXVariant((quint32)(context.Ebx)));
            g_statusCurrent.mapRegs.insert(XREG_ECX,XBinary::getXVariant((quint32)(context.Ecx)));
            g_statusCurrent.mapRegs.insert(XREG_EDX,XBinary::getXVariant((quint32)(context.Edx)));
            g_statusCurrent.mapRegs.insert(XREG_EBP,XBinary::getXVariant((quint32)(context.Ebp)));
            g_statusCurrent.mapRegs.insert(XREG_ESP,XBinary::getXVariant((quint32)(context.Esp)));
            g_statusCurrent.mapRegs.insert(XREG_ESI,XBinary::getXVariant((quint32)(context.Esi)));
            g_statusCurrent.mapRegs.insert(XREG_EDI,XBinary::getXVariant((quint32)(context.Edi)));
        #endif
        #ifdef Q_PROCESSOR_X86_64
            g_statusCurrent.mapRegs.insert(XREG_RAX,XBinary::getXVariant((quint64)(context.Rax)));
            g_statusCurrent.mapRegs.insert(XREG_RBX,XBinary::getXVariant((quint64)(context.Rbx)));
            g_statusCurrent.mapRegs.insert(XREG_RCX,XBinary::getXVariant((quint64)(context.Rcx)));
            g_statusCurrent.mapRegs.insert(XREG_RDX,XBinary::getXVariant((quint64)(context.Rdx)));
            g_statusCurrent.mapRegs.insert(XREG_RBP,XBinary::getXVariant((quint64)(context.Rbp)));
            g_statusCurrent.mapRegs.insert(XREG_RSP,XBinary::getXVariant((quint64)(context.Rsp)));
            g_statusCurrent.mapRegs.insert(XREG_RSI,XBinary::getXVariant((quint64)(context.Rsi)));
            g_statusCurrent.mapRegs.insert(XREG_RDI,XBinary::getXVariant((quint64)(context.Rdi)));
            g_statusCurrent.mapRegs.insert(XREG_R8,XBinary::getXVariant((quint64)(context.R8)));
            g_statusCurrent.mapRegs.insert(XREG_R9,XBinary::getXVariant((quint64)(context.R9)));
            g_statusCurrent.mapRegs.insert(XREG_R10,XBinary::getXVariant((quint64)(context.R10)));
            g_statusCurrent.mapRegs.insert(XREG_R11,XBinary::getXVariant((quint64)(context.R11)));
            g_statusCurrent.mapRegs.insert(XREG_R12,XBinary::getXVariant((quint64)(context.R12)));
            g_statusCurrent.mapRegs.insert(XREG_R13,XBinary::getXVariant((quint64)(context.R13)));
            g_statusCurrent.mapRegs.insert(XREG_R14,XBinary::getXVariant((quint64)(context.R14)));
            g_statusCurrent.mapRegs.insert(XREG_R15,XBinary::getXVariant((quint64)(context.R15)));
        #endif
        }

        if(regOptions.bIP)
        {
        #ifdef Q_PROCESSOR_X86_32
            g_statusCurrent.mapRegs.insert(XREG_EIP,XBinary::getXVariant((quint32)(context.Eip)));
        #endif
        #ifdef Q_PROCESSOR_X86_64
            g_statusCurrent.mapRegs.insert(XREG_RIP,XBinary::getXVariant((quint64)(context.Rip)));
        #endif
        }

        if(regOptions.bFlags)
        {
        #ifdef Q_PROCESSOR_X86_32
            g_statusCurrent.mapRegs.insert(XREG_EFLAGS,XBinary::getXVariant((quint32)(context.EFlags)));
        #endif
        #ifdef Q_PROCESSOR_X86_64
            g_statusCurrent.mapRegs.insert(XREG_RFLAGS,XBinary::getXVariant((quint64)(context.EFlags))); // TODO !!!
        #endif
        }

        if(regOptions.bSegments)
        {
            g_statusCurrent.mapRegs.insert(XREG_CS,XBinary::getXVariant((quint16)(context.SegCs)));
            g_statusCurrent.mapRegs.insert(XREG_FS,XBinary::getXVariant((quint16)(context.SegFs)));
            g_statusCurrent.mapRegs.insert(XREG_ES,XBinary::getXVariant((quint16)(context.SegEs)));
            g_statusCurrent.mapRegs.insert(XREG_DS,XBinary::getXVariant((quint16)(context.SegDs)));
            g_statusCurrent.mapRegs.insert(XREG_GS,XBinary::getXVariant((quint16)(context.SegGs)));
            g_statusCurrent.mapRegs.insert(XREG_SS,XBinary::getXVariant((quint16)(context.SegSs)));
        }

        if(regOptions.bDebug)
        {
        #ifdef Q_PROCESSOR_X86_32
            g_statusCurrent.mapRegs.insert(XREG_DR0,XBinary::getXVariant((quint32)(context.Dr0)));
            g_statusCurrent.mapRegs.insert(XREG_DR1,XBinary::getXVariant((quint32)(context.Dr1)));
            g_statusCurrent.mapRegs.insert(XREG_DR2,XBinary::getXVariant((quint32)(context.Dr2)));
            g_statusCurrent.mapRegs.insert(XREG_DR3,XBinary::getXVariant((quint32)(context.Dr3)));
            g_statusCurrent.mapRegs.insert(XREG_DR6,XBinary::getXVariant((quint32)(context.Dr6)));
            g_statusCurrent.mapRegs.insert(XREG_DR7,XBinary::getXVariant((quint32)(context.Dr7)));
        #endif
        #ifdef Q_PROCESSOR_X86_64
            g_statusCurrent.mapRegs.insert(XREG_DR0,XBinary::getXVariant((quint64)(context.Dr0)));
            g_statusCurrent.mapRegs.insert(XREG_DR1,XBinary::getXVariant((quint64)(context.Dr1)));
            g_statusCurrent.mapRegs.insert(XREG_DR2,XBinary::getXVariant((quint64)(context.Dr2)));
            g_statusCurrent.mapRegs.insert(XREG_DR3,XBinary::getXVariant((quint64)(context.Dr3)));
            g_statusCurrent.mapRegs.insert(XREG_DR6,XBinary::getXVariant((quint64)(context.Dr6)));
            g_statusCurrent.mapRegs.insert(XREG_DR7,XBinary::getXVariant((quint64)(context.Dr7)));
        #endif
        }

        if(regOptions.bFloat)
        {
        #if defined(Q_PROCESSOR_X86_64)
            for(qint32 i=0;i<8;i++)
            {
                g_statusCurrent.mapRegs.insert(XREG(XREG_ST0+i),XBinary::getXVariant((quint64)(context.FltSave.FloatRegisters[i].Low),(quint64)(context.FltSave.FloatRegisters[i].High)));
            }
        #endif
        }

        if(regOptions.bXMM)
        {
        #if defined(Q_PROCESSOR_X86_64)
            for(qint32 i=0;i<16;i++)
            {
                g_statusCurrent.mapRegs.insert(XREG(XREG_XMM0+i),XBinary::getXVariant((quint64)(context.FltSave.XmmRegisters[i].Low),(quint64)(context.FltSave.XmmRegisters[i].High)));
            }
        #endif
//            mapResult.insert("MxCsr",(quint32)(context.MxCsr));
        }

    #ifdef QT_DEBUG
     #if defined(Q_PROCESSOR_X86_64)
//        qDebug("P1Home %s",XBinary::valueToHex((quint64)(context.P1Home)).toLatin1().data());
//        qDebug("P2Home %s",XBinary::valueToHex((quint64)(context.P2Home)).toLatin1().data());
//        qDebug("P3Home %s",XBinary::valueToHex((quint64)(context.P3Home)).toLatin1().data());
//        qDebug("P4Home %s",XBinary::valueToHex((quint64)(context.P4Home)).toLatin1().data());
//        qDebug("P5Home %s",XBinary::valueToHex((quint64)(context.P5Home)).toLatin1().data());
//        qDebug("P6Home %s",XBinary::valueToHex((quint64)(context.P6Home)).toLatin1().data());
//        qDebug("ContextFlags %s",XBinary::valueToHex((quint32)(context.ContextFlags)).toLatin1().data());
//        qDebug("MxCsr %s",XBinary::valueToHex((quint32)(context.MxCsr)).toLatin1().data());

//        qDebug("DebugControl %s",XBinary::valueToHex((quint64)(context.DebugControl)).toLatin1().data());
//        qDebug("LastBranchToRip %s",XBinary::valueToHex((quint64)(context.LastBranchToRip)).toLatin1().data());
//        qDebug("LastBranchFromRip %s",XBinary::valueToHex((quint64)(context.LastBranchFromRip)).toLatin1().data());
//        qDebug("LastExceptionToRip %s",XBinary::valueToHex((quint64)(context.LastExceptionToRip)).toLatin1().data());
//        qDebug("LastExceptionFromRip %s",XBinary::valueToHex((quint64)(context.LastExceptionFromRip)).toLatin1().data());
     #endif
    #endif
    }
#endif
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateMemoryRegionsList()
{
    g_statusPrev.listMemoryRegions=g_statusCurrent.listMemoryRegions;

    g_statusCurrent.listMemoryRegions.clear();
#ifdef Q_OS_WIN
    g_statusCurrent.listMemoryRegions=XProcess::getMemoryRegionsListByHandle(g_processInfo.hProcess,0,0xFFFFFFFFFFFFFFFF);
#endif
#ifdef Q_OS_LINUX
    g_statusCurrent.listMemoryRegions=XProcess::getMemoryRegionsListByHandle(g_processInfo.hProcessMemoryQuery,0,0xFFFFFFFFFFFFFFFF);
#endif
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateModulesList()
{
    g_statusPrev.listModules=g_statusCurrent.listModules;

    g_statusCurrent.listModules.clear();

    g_statusCurrent.listModules=XProcess::getModulesList(g_processInfo.nProcessID);
}
#endif
#ifdef USE_XPROCESS
XBinary::XVARIANT XInfoDB::getCurrentRegCache(XREG reg)
{
    return _getRegCache(&(g_statusCurrent.mapRegs),reg);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentRegCache(XREG reg,XBinary::XVARIANT variant)
{
    _setRegCache(&(g_statusCurrent.mapRegs),reg,variant);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentRegByThread(X_HANDLE hThread,XREG reg,XBinary::XVARIANT variant)
{
    bool bResult=false;
#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_ALL; // All registers TODO Check regOptions |CONTEXT_FLOATING_POINT|CONTEXT_EXTENDED_REGISTERS

    if(GetThreadContext(hThread,&context))
    {
        bool bUnknownRegister=false;

    #ifdef Q_PROCESSOR_X86_32
        if      (reg==XREG_EAX)         context.Eax=variant.var.v_uint32;
        else if (reg==XREG_EBX)         context.Ebx=variant.var.v_uint32;
        else if (reg==XREG_ECX)         context.Ecx=variant.var.v_uint32;
        else if (reg==XREG_EDX)         context.Edx=variant.var.v_uint32;
        else if (reg==XREG_EBP)         context.Ebp=variant.var.v_uint32;
        else if (reg==XREG_ESP)         context.Esp=variant.var.v_uint32;
        else if (reg==XREG_ESI)         context.Esi=variant.var.v_uint32;
        else if (reg==XREG_EDI)         context.Edi=variant.var.v_uint32;
        else bUnknownRegister=true;
    #endif
    #ifdef Q_PROCESSOR_X86_64
        if      (reg==XREG_RAX)         context.Rax=variant.var.v_uint64;
        else if (reg==XREG_RBX)         context.Rbx=variant.var.v_uint64;
        else if (reg==XREG_RCX)         context.Rcx=variant.var.v_uint64;
        else if (reg==XREG_RDX)         context.Rdx=variant.var.v_uint64;
        else if (reg==XREG_RBP)         context.Rbp=variant.var.v_uint64;
        else if (reg==XREG_RSP)         context.Rsp=variant.var.v_uint64;
        else if (reg==XREG_RSI)         context.Rsi=variant.var.v_uint64;
        else if (reg==XREG_RDI)         context.Rdi=variant.var.v_uint64;
        else if (reg==XREG_R8)          context.R8=variant.var.v_uint64;
        else if (reg==XREG_R9)          context.R9=variant.var.v_uint64;
        else if (reg==XREG_R10)         context.R10=variant.var.v_uint64;
        else if (reg==XREG_R11)         context.R11=variant.var.v_uint64;
        else if (reg==XREG_R12)         context.R12=variant.var.v_uint64;
        else if (reg==XREG_R13)         context.R13=variant.var.v_uint64;
        else if (reg==XREG_R14)         context.R14=variant.var.v_uint64;
        else if (reg==XREG_R15)         context.R15=variant.var.v_uint64;
        else bUnknownRegister=true;
    #endif
        // TODO more

        if(!bUnknownRegister)
        {
            if(SetThreadContext(hThread,&context))
            {
                bResult=true;
            }
        }
        else
        {
        #ifdef QT_DEBUG
            qDebug("Unknown register");
        #endif
        }
    }
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentRegById(X_ID nThreadId,XREG reg,XBinary::XVARIANT variant)
{
    bool bResult=false;
#ifdef Q_OS_LINUX
    // TODO
    user_regs_struct regs={};

    errno=0;

    if(ptrace(PTRACE_GETREGS,nThreadId,nullptr,&regs)!=-1)
    {
        bool bUnknownRegister=false;
    #ifdef Q_PROCESSOR_X86_64
        if      (reg==XREG_RAX)         regs.rax=variant.var.v_uint64;
        else if (reg==XREG_RBX)         regs.rbx=variant.var.v_uint64;
        else if (reg==XREG_RCX)         regs.rcx=variant.var.v_uint64;
        else if (reg==XREG_RDX)         regs.rdx=variant.var.v_uint64;
        else if (reg==XREG_RBP)         regs.rbp=variant.var.v_uint64;
        else if (reg==XREG_RSP)         regs.rsp=variant.var.v_uint64;
        else if (reg==XREG_RSI)         regs.rsi=variant.var.v_uint64;
        else if (reg==XREG_RDI)         regs.rdi=variant.var.v_uint64;
        else if (reg==XREG_R8)          regs.r8=variant.var.v_uint64;
        else if (reg==XREG_R9)          regs.r9=variant.var.v_uint64;
        else if (reg==XREG_R10)         regs.r10=variant.var.v_uint64;
        else if (reg==XREG_R11)         regs.r11=variant.var.v_uint64;
        else if (reg==XREG_R12)         regs.r12=variant.var.v_uint64;
        else if (reg==XREG_R13)         regs.r13=variant.var.v_uint64;
        else if (reg==XREG_R14)         regs.r14=variant.var.v_uint64;
        else if (reg==XREG_R15)         regs.r15=variant.var.v_uint64;
        else bUnknownRegister=true;
    #endif

        if(!bUnknownRegister)
        {
            if(ptrace(PTRACE_SETREGS,nThreadId,nullptr,&regs)!=-1)
            {
                bResult=true;
            }
        }
    }
    else
    {
        qDebug("ptrace failed: %s",strerror(errno));
    }
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentReg(XREG reg,XBinary::XVARIANT variant)
{
    bool bResult=false;
#ifdef Q_OS_WIN
    bResult=setCurrentRegByThread(g_statusCurrent.hThread,reg,variant);
#endif
#ifdef Q_OS_LINUX
    bResult=setCurrentRegById(g_statusCurrent.nThreadId,reg,variant);
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
QList<XProcess::MEMORY_REGION> *XInfoDB::getCurrentMemoryRegionsList()
{
    return &(g_statusCurrent.listMemoryRegions);
}
#endif
#ifdef USE_XPROCESS
QList<XProcess::MODULE> *XInfoDB::getCurrentModulesList()
{
    return &(g_statusCurrent.listModules);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::addBreakPoint(quint64 nAddress,BPT bpType,BPI bpInfo,qint32 nCount,QString sInfo,QString sGUID)
{
    bool bResult=false;

    if(bpType==BPT_CODE_SOFTWARE)
    {
        if(!isBreakPointPresent(nAddress,bpType))
        {
            BREAKPOINT bp={};
            bp.nAddress=nAddress;
            bp.nSize=1;
            bp.nCount=nCount;
            bp.bpInfo=bpInfo;
            bp.bpType=bpType;
            bp.sInfo=sInfo;
            bp.sGUID=sGUID;

            bp.nOrigDataSize=1;

            if(read_array(nAddress,bp.origData,bp.nOrigDataSize)==bp.nOrigDataSize)
            {
                if(write_array(nAddress,(char *)"\xCC",bp.nOrigDataSize)) // TODO Check if x86
                {
                    g_listBreakpoints.append(bp);

                    bResult=true;
                }
            }
        }
    }
    else if(bpType==BPT_CODE_HARDWARE)
    {
        // TODO
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::removeBreakPoint(quint64 nAddress,BPT bpType)
{
    bool bResult=false;

    if(bpType==BPT_CODE_SOFTWARE)
    {
        BREAKPOINT bp=findBreakPointByAddress(nAddress,bpType);

        if(bp.nAddress==nAddress)
        {
            if(write_array(nAddress,(char *)bp.origData,bp.nOrigDataSize)) // TODO Check
            {
                bResult=true;
            }
        }
    }
    else if(bpType==XInfoDB::BPT_CODE_HARDWARE)
    {
        // TODO
    }

    if(bResult)
    {
        qint32 nNumberOfRecords=g_listBreakpoints.count();

        for(qint32 i=0;i<nNumberOfRecords;i++)
        {
            if((g_listBreakpoints.at(i).nAddress==nAddress)&&(g_listBreakpoints.at(i).bpType==bpType))
            {
                g_listBreakpoints.removeAt(i);

                break;
            }
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::isBreakPointPresent(quint64 nAddress,BPT bpType)
{
    bool bResult=false;

    BREAKPOINT bp=findBreakPointByAddress(nAddress,bpType);

    bResult=(bp.nAddress==nAddress);

    return bResult;
}
#endif
#ifdef USE_XPROCESS
QList<XInfoDB::BREAKPOINT> *XInfoDB::getBreakpoints()
{
    return &g_listBreakpoints;
}
#endif
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
QMap<X_HANDLE,XInfoDB::BREAKPOINT> *XInfoDB::getThreadBreakpoints()
{
    return &g_mapThreadBreakpoints;
}
#endif
#ifdef Q_OS_LINUX
QMap<X_ID,XInfoDB::BREAKPOINT> *XInfoDB::getThreadBreakpoints()
{
    return &g_mapThreadBreakpoints;
}
#endif
#endif
#ifdef USE_XPROCESS
bool XInfoDB::isRegChanged(XREG reg)
{
    return !(XBinary::isXVariantEqual(_getRegCache(&(g_statusCurrent.mapRegs),reg),_getRegCache(&(g_statusPrev.mapRegs),reg)));
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentStackPointerCache()
{
    XADDR nResult=0;

#ifdef Q_PROCESSOR_X86_32
    nResult=getCurrentRegCache(XInfoDB::XREG_ESP).var.v_uint32;
#endif
#ifdef Q_PROCESSOR_X86_64
    nResult=getCurrentRegCache(XInfoDB::XREG_RSP).var.v_uint64;
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentInstructionPointerCache()
{
    XADDR nResult=0;

#ifdef Q_PROCESSOR_X86_32
    nResult=getCurrentRegCache(XInfoDB::XREG_EIP).var.v_uint32;
#endif
#ifdef Q_PROCESSOR_X86_64
    nResult=getCurrentRegCache(XInfoDB::XREG_RIP).var.v_uint64;
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentInstructionPointerByHandle(X_HANDLE hThread)
{
    XADDR nResult=0;
#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_CONTROL; // EIP

    if(GetThreadContext(hThread,&context))
    {
#ifdef Q_PROCESSOR_X86_32
        nResult=context.Eip;
#endif
#ifdef Q_PROCESSOR_X86_64
        nResult=context.Rip;
#endif
    }
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentInstructionPointerById(X_ID nThreadId)
{
    XADDR nResult=0;
#ifdef Q_OS_LINUX
    user_regs_struct regs={};

    errno=0;

    if(ptrace(PTRACE_GETREGS,nThreadId,nullptr,&regs)!=-1)
    {
    #if defined(Q_PROCESSOR_X86_64)
        nResult=regs.rip;
    #elif defined(Q_PROCESSOR_X86_32)
        nResult=regs.eip;
    #endif
    }
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentIntructionPointerByHandle(X_HANDLE hThread,XADDR nValue)
{
    bool bResult=false;
#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_CONTROL; // EIP

    if(GetThreadContext(hThread,&context))
    {
#ifdef Q_PROCESSOR_X86_32
        context.Eip=nValue;
#endif
#ifdef Q_PROCESSOR_X86_64
        context.Rip=nValue;
#endif
        if(SetThreadContext(hThread,&context))
        {
            bResult=true;
        }
    }
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
XCapstone::OPCODE_ID XInfoDB::getCurrentOpcodeByHandle(X_HANDLE hThread)
{
    XCapstone::OPCODE_ID result={};

    // TODO

    return result;
}
#endif
#ifdef USE_XPROCESS
XCapstone::OPCODE_ID XInfoDB::getCurrentOpcodeById(X_ID nThreadId)
{
    XCapstone::OPCODE_ID result={};

    // TODO

    return result;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentStackPointerByHandle(X_HANDLE hThread)
{
    XADDR nResult=0;
#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_CONTROL;

    if(GetThreadContext(hThread,&context))
    {
    #ifdef Q_PROCESSOR_X86_32
        nResult=(quint32)(context.Esp);
    #endif
    #ifdef Q_PROCESSOR_X86_64
        nResult=(quint64)(context.Rsp);
    #endif
    }
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentStackPointerById(X_ID nThreadId)
{
    XADDR nResult=0;

    // TODO

    return nResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentStackPointerByHandle(X_HANDLE hThread,XADDR nValue)
{
    bool bResult=false;

    // TODO

    return bResult;
}
#endif
//#ifdef USE_XPROCESS
//void XInfoDB::_lockId(quint32 nId)
//{
//    QMutex *pMutex=nullptr;

//    if(g_mapIds.contains(nId))
//    {
//        pMutex=g_mapIds.value(nId);
//    }
//    else
//    {
//        pMutex=new QMutex;
//        g_mapIds.insert(nId,pMutex);
//    }

//    if(pMutex)
//    {
//        pMutex->lock();
//    }
//}
//#endif
//#ifdef USE_XPROCESS
//void XInfoDB::_unlockID(quint32 nId)
//{
//    if(g_mapIds.contains(nId))
//    {
//        QMutex *pMutex=g_mapIds.value(nId);

//        pMutex->unlock();
//    }
//}
//#endif
//#ifdef USE_XPROCESS
//void XInfoDB::_waitID(quint32 nId)
//{
//    if(g_mapIds.contains(nId))
//    {
//        QMutex *pMutex=g_mapIds.value(nId);

//        pMutex->lock();
//        qDebug("TEST");
//        pMutex->unlock();
//    }
//}
//#endif

QList<XBinary::MEMORY_REPLACE> XInfoDB::getMemoryReplaces(quint64 nBase,quint64 nSize)
{
    QList<XBinary::MEMORY_REPLACE> listResult;
#ifdef USE_XPROCESS
    qint32 nNumberOfRecords=g_listBreakpoints.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        XInfoDB::BREAKPOINT breakPoint=g_listBreakpoints.at(i);

        if(breakPoint.nOrigDataSize)
        {
            if((breakPoint.nAddress>=nBase)&&(breakPoint.nAddress<nBase+nSize))
            {
                XBinary::MEMORY_REPLACE record={};
                record.nAddress=breakPoint.nAddress;
                record.baOriginal=QByteArray(breakPoint.origData,breakPoint.nOrigDataSize);
                record.nSize=record.baOriginal.size();

                listResult.append(record);
            }
        }
    }
#endif
    return listResult;
}
#ifdef USE_XPROCESS
QString XInfoDB::regIdToString(XREG reg)
{
    QString sResult="Unknown";

    if      (reg==XREG_NONE)        sResult=QString("");
#ifdef Q_PROCESSOR_X86
    else if (reg==XREG_AX)          sResult=QString("AX");
    else if (reg==XREG_CX)          sResult=QString("CX");
    else if (reg==XREG_DX)          sResult=QString("DX");
    else if (reg==XREG_BX)          sResult=QString("BX");
    else if (reg==XREG_SP)          sResult=QString("SP");
    else if (reg==XREG_BP)          sResult=QString("BP");
    else if (reg==XREG_SI)          sResult=QString("SI");
    else if (reg==XREG_DI)          sResult=QString("DI");
    else if (reg==XREG_IP)          sResult=QString("IP");
    else if (reg==XREG_FLAGS)       sResult=QString("FLAGS");
    else if (reg==XREG_EAX)         sResult=QString("EAX");
    else if (reg==XREG_ECX)         sResult=QString("ECX");
    else if (reg==XREG_EDX)         sResult=QString("EDX");
    else if (reg==XREG_EBX)         sResult=QString("EBX");
    else if (reg==XREG_ESP)         sResult=QString("ESP");
    else if (reg==XREG_EBP)         sResult=QString("EBP");
    else if (reg==XREG_ESI)         sResult=QString("ESI");
    else if (reg==XREG_EDI)         sResult=QString("EDI");
    else if (reg==XREG_EIP)         sResult=QString("EIP");
    else if (reg==XREG_EFLAGS)      sResult=QString("EFLAGS");
#ifdef Q_PROCESSOR_X86_64
    else if (reg==XREG_RAX)         sResult=QString("RAX");
    else if (reg==XREG_RCX)         sResult=QString("RCX");
    else if (reg==XREG_RDX)         sResult=QString("RDX");
    else if (reg==XREG_RBX)         sResult=QString("RBX");
    else if (reg==XREG_RSP)         sResult=QString("RSP");
    else if (reg==XREG_RBP)         sResult=QString("RBP");
    else if (reg==XREG_RSI)         sResult=QString("RSI");
    else if (reg==XREG_RDI)         sResult=QString("RDI");
    else if (reg==XREG_R8)          sResult=QString("R8");
    else if (reg==XREG_R9)          sResult=QString("R9");
    else if (reg==XREG_R10)         sResult=QString("R10");
    else if (reg==XREG_R11)         sResult=QString("R11");
    else if (reg==XREG_R12)         sResult=QString("R12");
    else if (reg==XREG_R13)         sResult=QString("R13");
    else if (reg==XREG_R14)         sResult=QString("R14");
    else if (reg==XREG_R15)         sResult=QString("R15");
    else if (reg==XREG_RIP)         sResult=QString("RIP");
    else if (reg==XREG_RFLAGS)      sResult=QString("RFLAGS");
#endif
    else if (reg==XREG_CS)          sResult=QString("CS");
    else if (reg==XREG_DS)          sResult=QString("DS");
    else if (reg==XREG_ES)          sResult=QString("ES");
    else if (reg==XREG_FS)          sResult=QString("FS");
    else if (reg==XREG_GS)          sResult=QString("GS");
    else if (reg==XREG_SS)          sResult=QString("SS");
    else if (reg==XREG_DR0)         sResult=QString("DR0");
    else if (reg==XREG_DR1)         sResult=QString("DR1");
    else if (reg==XREG_DR2)         sResult=QString("DR2");
    else if (reg==XREG_DR3)         sResult=QString("DR3");
    else if (reg==XREG_DR6)         sResult=QString("DR6");
    else if (reg==XREG_DR7)         sResult=QString("DR7");
    else if (reg==XREG_CF)          sResult=QString("CF");
    else if (reg==XREG_PF)          sResult=QString("PF");
    else if (reg==XREG_AF)          sResult=QString("AF");
    else if (reg==XREG_ZF)          sResult=QString("ZF");
    else if (reg==XREG_SF)          sResult=QString("SF");
    else if (reg==XREG_TF)          sResult=QString("TF");
    else if (reg==XREG_IF)          sResult=QString("IF");
    else if (reg==XREG_DF)          sResult=QString("DF");
    else if (reg==XREG_OF)          sResult=QString("OF");
    else if (reg==XREG_ST0)         sResult=QString("ST0");
    else if (reg==XREG_ST1)         sResult=QString("ST1");
    else if (reg==XREG_ST2)         sResult=QString("ST2");
    else if (reg==XREG_ST3)         sResult=QString("ST3");
    else if (reg==XREG_ST4)         sResult=QString("ST4");
    else if (reg==XREG_ST5)         sResult=QString("ST5");
    else if (reg==XREG_ST6)         sResult=QString("ST6");
    else if (reg==XREG_ST7)         sResult=QString("ST7");
    else if (reg==XREG_XMM0)        sResult=QString("XMM0");
    else if (reg==XREG_XMM1)        sResult=QString("XMM1");
    else if (reg==XREG_XMM2)        sResult=QString("XMM2");
    else if (reg==XREG_XMM3)        sResult=QString("XMM3");
    else if (reg==XREG_XMM4)        sResult=QString("XMM4");
    else if (reg==XREG_XMM5)        sResult=QString("XMM5");
    else if (reg==XREG_XMM6)        sResult=QString("XMM6");
    else if (reg==XREG_XMM7)        sResult=QString("XMM7");
    else if (reg==XREG_XMM8)        sResult=QString("XMM8");
    else if (reg==XREG_XMM9)        sResult=QString("XMM9");
    else if (reg==XREG_XMM10)       sResult=QString("XMM10");
    else if (reg==XREG_XMM11)       sResult=QString("XMM11");
    else if (reg==XREG_XMM12)       sResult=QString("XMM12");
    else if (reg==XREG_XMM13)       sResult=QString("XMM13");
    else if (reg==XREG_XMM14)       sResult=QString("XMM14");
    else if (reg==XREG_XMM15)       sResult=QString("XMM15");
    else if (reg==XREG_AH)          sResult=QString("AH");
    else if (reg==XREG_CH)          sResult=QString("CH");
    else if (reg==XREG_DH)          sResult=QString("DH");
    else if (reg==XREG_BH)          sResult=QString("BH");
    else if (reg==XREG_AL)          sResult=QString("AL");
    else if (reg==XREG_CL)          sResult=QString("CL");
    else if (reg==XREG_DL)          sResult=QString("DL");
    else if (reg==XREG_BL)          sResult=QString("BL");
#ifdef Q_PROCESSOR_X86_64
    else if (reg==XREG_SPL)         sResult=QString("SPL");
    else if (reg==XREG_BPL)         sResult=QString("BPL");
    else if (reg==XREG_SIL)         sResult=QString("SIL");
    else if (reg==XREG_DIL)         sResult=QString("DIL");
    else if (reg==XREG_R8D)         sResult=QString("R8D");
    else if (reg==XREG_R9D)         sResult=QString("R9D");
    else if (reg==XREG_R10D)        sResult=QString("R10D");
    else if (reg==XREG_R11D)        sResult=QString("R11D");
    else if (reg==XREG_R12D)        sResult=QString("R12D");
    else if (reg==XREG_R13D)        sResult=QString("R13D");
    else if (reg==XREG_R14D)        sResult=QString("R14D");
    else if (reg==XREG_R15D)        sResult=QString("R15D");
    else if (reg==XREG_R8W)         sResult=QString("R8W");
    else if (reg==XREG_R9W)         sResult=QString("R9W");
    else if (reg==XREG_R10W)        sResult=QString("R10W");
    else if (reg==XREG_R11W)        sResult=QString("R11W");
    else if (reg==XREG_R12W)        sResult=QString("R12W");
    else if (reg==XREG_R13W)        sResult=QString("R13W");
    else if (reg==XREG_R14W)        sResult=QString("R14W");
    else if (reg==XREG_R15W)        sResult=QString("R15W");
    else if (reg==XREG_R8B)         sResult=QString("R8B");
    else if (reg==XREG_R9B)         sResult=QString("R9B");
    else if (reg==XREG_R10B)        sResult=QString("R10B");
    else if (reg==XREG_R11B)        sResult=QString("R11B");
    else if (reg==XREG_R12B)        sResult=QString("R12B");
    else if (reg==XREG_R13B)        sResult=QString("R13B");
    else if (reg==XREG_R14B)        sResult=QString("R14B");
    else if (reg==XREG_R15B)        sResult=QString("R15B");
#endif
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg32(XREG reg)
{
    XREG result=XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_64
    if      (reg==XREG_RAX)         result=XREG_EAX;
    else if (reg==XREG_RCX)         result=XREG_ECX;
    else if (reg==XREG_RDX)         result=XREG_EDX;
    else if (reg==XREG_RBX)         result=XREG_EBX;
    else if (reg==XREG_RSP)         result=XREG_ESP;
    else if (reg==XREG_RBP)         result=XREG_EBP;
    else if (reg==XREG_RSI)         result=XREG_ESI;
    else if (reg==XREG_RDI)         result=XREG_EDI;
    else if (reg==XREG_R8)          result=XREG_R8D;
    else if (reg==XREG_R9)          result=XREG_R9D;
    else if (reg==XREG_R10)         result=XREG_R10D;
    else if (reg==XREG_R11)         result=XREG_R11D;
    else if (reg==XREG_R12)         result=XREG_R12D;
    else if (reg==XREG_R13)         result=XREG_R13D;
    else if (reg==XREG_R14)         result=XREG_R14D;
    else if (reg==XREG_R15)         result=XREG_R15D;
    else if (reg==XREG_RIP)         result=XREG_EIP;
    else if (reg==XREG_RFLAGS)      result=XREG_EFLAGS;
#endif
#endif
    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg16(XREG reg)
{
    XREG result=XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
    if      (reg==XREG_EAX)             result=XREG_AX;
    else if (reg==XREG_ECX)             result=XREG_CX;
    else if (reg==XREG_EDX)             result=XREG_DX;
    else if (reg==XREG_EBX)             result=XREG_BX;
    else if (reg==XREG_ESP)             result=XREG_SP;
    else if (reg==XREG_EBP)             result=XREG_BP;
    else if (reg==XREG_ESI)             result=XREG_SI;
    else if (reg==XREG_EDI)             result=XREG_DI;
    else if (reg==XREG_EIP)             result=XREG_IP;
    else if (reg==XREG_EFLAGS)          result=XREG_FLAGS;
#endif
#ifdef Q_PROCESSOR_X86_64
    if      ((reg==XREG_RAX)||(reg==XREG_EAX))          result=XREG_AX;
    else if ((reg==XREG_RCX)||(reg==XREG_ECX))          result=XREG_CX;
    else if ((reg==XREG_RDX)||(reg==XREG_EDX))          result=XREG_DX;
    else if ((reg==XREG_RBX)||(reg==XREG_EBX))          result=XREG_BX;
    else if ((reg==XREG_RSP)||(reg==XREG_ESP))          result=XREG_SP;
    else if ((reg==XREG_RBP)||(reg==XREG_EBP))          result=XREG_BP;
    else if ((reg==XREG_RSI)||(reg==XREG_ESI))          result=XREG_SI;
    else if ((reg==XREG_RDI)||(reg==XREG_EDI))          result=XREG_DI;
    else if ((reg==XREG_R8)||(reg==XREG_R8D))           result=XREG_R8W;
    else if ((reg==XREG_R9)||(reg==XREG_R9D))           result=XREG_R9W;
    else if ((reg==XREG_R10)||(reg==XREG_R10D))         result=XREG_R10W;
    else if ((reg==XREG_R11)||(reg==XREG_R11D))         result=XREG_R11W;
    else if ((reg==XREG_R12)||(reg==XREG_R12D))         result=XREG_R12W;
    else if ((reg==XREG_R13)||(reg==XREG_R13D))         result=XREG_R13W;
    else if ((reg==XREG_R14)||(reg==XREG_R14D))         result=XREG_R14W;
    else if ((reg==XREG_R15)||(reg==XREG_R15D))         result=XREG_R15W;
    else if ((reg==XREG_RIP)||(reg==XREG_EIP))          result=XREG_IP;
    else if ((reg==XREG_RFLAGS)||(reg==XREG_EFLAGS))    result=XREG_FLAGS;
#endif
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg8H(XREG reg)
{
    XREG result=XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
    if      ((reg==XREG_EAX)||(reg==XREG_AX))       result=XREG_AH;
    else if ((reg==XREG_ECX)||(reg==XREG_CX))       result=XREG_CH;
    else if ((reg==XREG_EDX)||(reg==XREG_DX))       result=XREG_DH;
    else if ((reg==XREG_EBX)||(reg==XREG_BX))       result=XREG_BH;
#endif
#ifdef Q_PROCESSOR_X86_64
    if      ((reg==XREG_RAX)||(reg==XREG_EAX)||(reg==XREG_AX))      result=XREG_AH;
    else if ((reg==XREG_RCX)||(reg==XREG_ECX)||(reg==XREG_CX))      result=XREG_CH;
    else if ((reg==XREG_RDX)||(reg==XREG_EDX)||(reg==XREG_DX))      result=XREG_DH;
    else if ((reg==XREG_RBX)||(reg==XREG_EBX)||(reg==XREG_BX))      result=XREG_BH;
#endif
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg8L(XREG reg)
{
    XREG result=XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
    if      ((reg==XREG_EAX)||(reg==XREG_AX))       result=XREG_AL;
    else if ((reg==XREG_ECX)||(reg==XREG_CX))       result=XREG_CL;
    else if ((reg==XREG_EDX)||(reg==XREG_DX))       result=XREG_DL;
    else if ((reg==XREG_EBX)||(reg==XREG_BX))       result=XREG_BL;
#endif
#ifdef Q_PROCESSOR_X86_64
    if      ((reg==XREG_RAX)||(reg==XREG_EAX)||(reg==XREG_AX))          result=XREG_AL;
    else if ((reg==XREG_RCX)||(reg==XREG_ECX)||(reg==XREG_CX))          result=XREG_CL;
    else if ((reg==XREG_RDX)||(reg==XREG_EDX)||(reg==XREG_DX))          result=XREG_DL;
    else if ((reg==XREG_RBX)||(reg==XREG_EBX)||(reg==XREG_BX))          result=XREG_BL;
    else if ((reg==XREG_RSP)||(reg==XREG_ESP)||(reg==XREG_SP))          result=XREG_SPL;
    else if ((reg==XREG_RBP)||(reg==XREG_EBP)||(reg==XREG_BP))          result=XREG_BPL;
    else if ((reg==XREG_RSI)||(reg==XREG_ESI)||(reg==XREG_SI))          result=XREG_SIL;
    else if ((reg==XREG_RDI)||(reg==XREG_EDI)||(reg==XREG_DI))          result=XREG_DIL;
    else if ((reg==XREG_R8)||(reg==XREG_R8D)||(reg==XREG_R8W))          result=XREG_R8B;
    else if ((reg==XREG_R9)||(reg==XREG_R9D)||(reg==XREG_R9W))          result=XREG_R9B;
    else if ((reg==XREG_R10)||(reg==XREG_R10D)||(reg==XREG_R10W))       result=XREG_R10B;
    else if ((reg==XREG_R11)||(reg==XREG_R11D)||(reg==XREG_R11W))       result=XREG_R11B;
    else if ((reg==XREG_R12)||(reg==XREG_R12D)||(reg==XREG_R12W))       result=XREG_R12B;
    else if ((reg==XREG_R13)||(reg==XREG_R13D)||(reg==XREG_R13W))       result=XREG_R13B;
    else if ((reg==XREG_R14)||(reg==XREG_R14D)||(reg==XREG_R14W))       result=XREG_R14B;
    else if ((reg==XREG_R15)||(reg==XREG_R15D)||(reg==XREG_R15W))       result=XREG_R15B;
#endif
#endif

    return result;
}
#endif
XInfoDB::RECORD_INFO XInfoDB::getRecordInfo(quint64 nValue,RI_TYPE riType)
{
    RECORD_INFO result={};

    if((nValue>=g_nMainModuleAddress)&&(nValue<(g_nMainModuleAddress+g_nMainModuleSize)))
    {
        result.bValid=true;
        result.sModule=g_sMainModuleName;
        result.nAddress=nValue;
    }
#ifdef USE_XPROCESS
    else
    {
        // TODO mapRegions
        XProcess::MEMORY_REGION memoryRegion=XProcess::getMemoryRegionByAddress(&(g_statusCurrent.listMemoryRegions),nValue);

        if(memoryRegion.nSize)
        {
            result.bValid=true;
            result.nAddress=nValue;

            XProcess::MODULE _module=XProcess::getModuleByAddress(&(g_statusCurrent.listModules),nValue);

            if(_module.nSize)
            {
                result.sModule=_module.sName;
            }
        }
    }
#endif

    if( (riType==RI_TYPE_GENERAL)||
        (riType==RI_TYPE_DATA)||
        (riType==RI_TYPE_ANSI)||
        (riType==RI_TYPE_UNICODE)||
        (riType==RI_TYPE_UTF8))
    {
        if(result.bValid)
        {
            result.baData=read_array(result.nAddress,64); // TODO const
        }
    }

    if( (riType==RI_TYPE_GENERAL)||
        (riType==RI_TYPE_SYMBOL))
    {
        if(result.bValid)
        {
            // TODO getSymbol
            // TODO
            // If not use address
            if(riType==RI_TYPE_SYMBOL)
            {
                if(result.sSymbol=="")
                {
                    result.sSymbol=QString("<%1.%2>").arg(result.sModule,XBinary::valueToHexOS(result.nAddress));
                }
            }
        }
    }

    return result;
}

QString XInfoDB::recordInfoToString(RECORD_INFO recordInfo,RI_TYPE riType)
{
    QString sResult;

    if(recordInfo.bValid)
    {
        if(riType==RI_TYPE_GENERAL)
        {
            QString sAnsiString;
            QString sUnicodeString;

            quint8 nAnsiSymbol=XBinary::_read_uint8(recordInfo.baData.data());
            quint16 nUnicodeSymbol=XBinary::_read_uint16(recordInfo.baData.data());

            if((nAnsiSymbol>=8)&&(nAnsiSymbol<128))
            {
                sAnsiString=QString::fromLatin1(recordInfo.baData);
            }

            if((nUnicodeSymbol>=8)&&(nUnicodeSymbol<128))
            {
                sUnicodeString=QString::fromUtf16((quint16 *)(recordInfo.baData.data()),recordInfo.baData.size()/2);
            }

            qint32 nAnsiSize=sAnsiString.size();
            qint32 nUnicodeSize=sUnicodeString.size();

            if((nAnsiSize>=nUnicodeSize)&&(nAnsiSize>5))
            {
                sResult=QString("A: \"%1\"").arg(sAnsiString);
            }
            else if((nUnicodeSize>=nAnsiSize)&&(nUnicodeSize>5))
            {
                sResult=QString("U: \"%1\"").arg(sUnicodeString);
            }
            else if(recordInfo.sSymbol!="")
            {
                sResult=recordInfo.sSymbol;
            }
            else
            {
                sResult=QString("<%1.%2>").arg(recordInfo.sModule,XBinary::valueToHexOS(recordInfo.nAddress));
            }
        }
        else if(riType==RI_TYPE_ADDRESS)
        {
            sResult=QString("<%1.%2>").arg(recordInfo.sModule,XBinary::valueToHexOS(recordInfo.nAddress));
        }
        else if(riType==RI_TYPE_SYMBOL)
        {
            sResult=recordInfo.sSymbol;
        }
        else if(riType==RI_TYPE_ANSI)
        {
            sResult=QString::fromLatin1(recordInfo.baData);
        }
        else if(riType==RI_TYPE_UNICODE)
        {
            sResult=QString::fromUtf16((quint16 *)(recordInfo.baData.data()),recordInfo.baData.size()/2);
        }
        else if(riType==RI_TYPE_UTF8)
        {
            sResult=QString::fromUtf8(recordInfo.baData);
        }
        else if(riType==RI_TYPE_SYMBOLADDRESS)
        {
            sResult=recordInfoToString(recordInfo,RI_TYPE_SYMBOL);

            if(sResult=="")
            {
                sResult=recordInfoToString(recordInfo,RI_TYPE_ADDRESS);
            }
        }
    }

    return sResult;
}

void XInfoDB::clearRecordInfoCache()
{
    g_mapSRecordInfoCache.clear();
}

XInfoDB::RECORD_INFO XInfoDB::getRecordInfoCache(quint64 nValue)
{
    RECORD_INFO result={};

    if(g_mapSRecordInfoCache.contains(nValue))
    {
        result=g_mapSRecordInfoCache.value(nValue);
    }
    else
    {
        result=getRecordInfo(nValue,RI_TYPE_GENERAL);

        g_mapSRecordInfoCache.insert(nValue,result);
    }

    return result;
}

QList<XInfoDB::SYMBOL> *XInfoDB::getSymbols()
{
    return &g_listSymbols;
}

QMap<quint32,QString> *XInfoDB::getSymbolModules()
{
    return &g_mapSymbolModules;
}

void XInfoDB::addSymbol(XADDR nAddress,qint64 nSize,quint32 nModule,QString sSymbol,ST symbolType,SS symbolSource)
{
    qint32 nInsertIndex=0;
    qint32 nIndex=_getSymbolIndex(nAddress,nSize,nModule,&nInsertIndex);

    if(nIndex!=-1)
    {
        g_listSymbols[nIndex].sSymbol=sSymbol;
        g_listSymbols[nIndex].symbolType=symbolType;
    }
    else
    {
        SYMBOL symbol={};
        symbol.nAddress=nAddress;
        symbol.nSize=nSize;
        symbol.nModule=nModule;
        symbol.sSymbol=sSymbol;
        symbol.symbolType=symbolType;
        symbol.symbolSource=symbolSource;

        g_listSymbols.insert(nInsertIndex,symbol);
    }
}

void XInfoDB::_addSymbol(XADDR nAddress,qint64 nSize,quint32 nModule,QString sSymbol,ST symbolType,SS symbolSource)
{
    SYMBOL symbol={};
    symbol.nAddress=nAddress;
    symbol.nSize=nSize;
    symbol.nModule=nModule;
    symbol.sSymbol=sSymbol;
    symbol.symbolType=symbolType;
    symbol.symbolSource=symbolSource;

    g_listSymbols.append(symbol);
}

void XInfoDB::_sortSymbols()
{
    std::sort(g_listSymbols.begin(),g_listSymbols.end(),_symbolSort);
}

qint32 XInfoDB::_getSymbolIndex(XADDR nAddress,qint64 nSize,quint32 nModule,qint32 *pnInsertIndex)
{
    // For sorted g_listSymbols!
    qint32 nResult=-1;

    qint32 nNumberOfRecords=g_listSymbols.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if((g_listSymbols.at(i).nAddress==nAddress)&&(g_listSymbols.at(i).nSize==nSize)&&(g_listSymbols.at(i).nModule==nModule))
        {
            nResult=i;

            break;
        }
        else if(g_listSymbols.at(i).nAddress<nAddress)
        {
            *pnInsertIndex=i;

            break;
        }
    }

    return nResult;
}

QString XInfoDB::symbolSourceIdToString(SS symbolSource)
{
    QString sResult=tr("Unknown");

    if      (symbolSource==SS_FILE)         sResult=tr("File");
    else if (symbolSource==SS_USER)         sResult=tr("User");

    return sResult;
}

QString XInfoDB::symbolTypeIdToString(ST symbolType)
{
    QString sResult=tr("Unknown");

    if      (symbolType==ST_LABEL)          sResult=tr("Label");
    else if (symbolType==ST_ENTRYPOINT)     sResult=tr("Entry point");
    else if (symbolType==ST_EXPORT)         sResult=tr("Export");
    else if (symbolType==ST_IMPORT)         sResult=tr("Import");
    else if (symbolType==ST_DATA)           sResult=tr("Data");
    else if (symbolType==ST_OBJECT)         sResult=tr("Object");
    else if (symbolType==ST_FUNCTION)       sResult=tr("Function");

    return sResult;
}

void XInfoDB::testFunction()
{
#ifdef USE_XPROCESS
#ifdef Q_OS_LINUX
    user_regs_struct regs={};

    errno=0;

    long int nRet=ptrace(PTRACE_GETREGS,g_statusCurrent.nThreadId,nullptr,&regs);

    qDebug("ptrace failed: %s",strerror(errno));

    if(nRet!=-1)
    {
        qDebug("TODO");
    }
    else
    {
        qDebug("PTRACE_GETREGS error");
    }
#endif
#endif
}
#ifdef USE_XPROCESS
XBinary::XVARIANT XInfoDB::_getRegCache(QMap<XREG,XBinary::XVARIANT> *pMapRegs,XREG reg)
{
    // TODO AX AL AH
    XBinary::XVARIANT result={};

    XREG _reg=reg;
#ifdef Q_PROCESSOR_X86
    if( (reg==XREG_CF)||(reg==XREG_PF)||(reg==XREG_AF)||
        (reg==XREG_ZF)||(reg==XREG_SF)||(reg==XREG_TF)||
        (reg==XREG_IF)||(reg==XREG_DF)||(reg==XREG_OF))
    {   
    #ifdef Q_PROCESSOR_X86_32
        _reg=XREG_EFLAGS;
    #endif
    #ifdef Q_PROCESSOR_X86_64
        _reg=XREG_RFLAGS;
    #endif
    }
#endif
#ifdef Q_PROCESSOR_X86_32
    // TODO
#endif
#ifdef Q_PROCESSOR_X86_64
    // TODO
#endif

    result=pMapRegs->value(_reg);

    if(result.mode!=XBinary::MODE_UNKNOWN)
    {
    #ifdef Q_PROCESSOR_X86
        if      (reg==XREG_CF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0001));
        else if (reg==XREG_PF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0004));
        else if (reg==XREG_AF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0010));
        else if (reg==XREG_ZF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0040));
        else if (reg==XREG_SF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0080));
        else if (reg==XREG_TF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0100));
        else if (reg==XREG_IF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0200));
        else if (reg==XREG_DF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0400));
        else if (reg==XREG_OF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0800));
    #endif
    }

    return result;
}
#ifdef USE_XPROCESS
#endif
void XInfoDB::_setRegCache(QMap<XREG,XBinary::XVARIANT> *pMapRegs,XREG reg,XBinary::XVARIANT variant)
{
    pMapRegs->insert(reg,variant);
}
#endif
