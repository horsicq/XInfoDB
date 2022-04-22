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

XInfoDB::XInfoDB(QObject *pParent) : QObject(pParent)
{
    g_mode=MODE_UNKNOWN;
#ifdef USE_XPROCESS
    g_processInfo={};
    g_hidThread={};
#endif
}

XInfoDB::~XInfoDB()
{
}

void XInfoDB::reload(bool bDataReload)
{
    emit dataChanged(bDataReload);
}

quint32 XInfoDB::read_uint32(quint64 nAddress,bool bIsBigEndian)
{
    quint32 nResult=0;
#ifdef USE_XPROCESS
    nResult=XProcess::read_uint32(g_processInfo.hProcessMemoryIO,nAddress,bIsBigEndian);
#endif
    return nResult;
}

quint64 XInfoDB::read_uint64(quint64 nAddress, bool bIsBigEndian)
{
    quint64 nResult=0;
#ifdef USE_XPROCESS
    nResult=XProcess::read_uint64(g_processInfo.hProcessMemoryIO,nAddress,bIsBigEndian);
#endif
    return nResult;
}

qint64 XInfoDB::read_array(quint64 nAddress,char *pData,quint64 nSize)
{
    qint64 nResult=0;
#ifdef USE_XPROCESS
    nResult=XProcess::read_array(g_processInfo.hProcessMemoryIO,nAddress,pData,nSize);
#endif
    return nResult;
}

qint64 XInfoDB::write_array(quint64 nAddress,char *pData,quint64 nSize)
{
    qint64 nResult=0;
#ifdef USE_XPROCESS
    nResult=XProcess::write_array(g_processInfo.hProcessMemoryIO,nAddress,pData,nSize);
#endif
    return nResult;
}

QByteArray XInfoDB::read_array(quint64 nAddress, quint64 nSize)
{
    QByteArray baResult;
#ifdef USE_XPROCESS
    baResult=XProcess::read_array(g_processInfo.hProcessMemoryIO,nAddress,nSize);
#endif
    return baResult;
}

QString XInfoDB::read_ansiString(quint64 nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef USE_XPROCESS
    sResult=XProcess::read_ansiString(g_processInfo.hProcessMemoryIO,nAddress,nMaxSize);
#endif
    return sResult;
}

QString XInfoDB::read_unicodeString(quint64 nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef USE_XPROCESS
    sResult=XProcess::read_unicodeString(g_processInfo.hProcessMemoryIO,nAddress,nMaxSize);
#endif
    return sResult;
}

QString XInfoDB::read_utf8String(quint64 nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef USE_XPROCESS
    sResult=XProcess::read_utf8String(g_processInfo.hProcessMemoryIO,nAddress,nMaxSize);
#endif
    return sResult;
}

#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByAddress(quint64 nAddress,BPT bpType)
{
    BREAKPOINT result={};

    if(bpType==BPT_CODE_SOFTWARE)
    {
        result=g_mapSoftwareBreakpoints.value(nAddress);
    }
    else if(bpType==BPT_CODE_HARDWARE)
    {
        result=g_mapHardwareBreakpoints.value(nAddress);
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByExceptionAddress(quint64 nExceptionAddress, BPT bpType)
{
    BREAKPOINT result={};

    QMapIterator<quint64,XInfoDB::BREAKPOINT> i(*getSoftwareBreakpoints());
    while (i.hasNext())
    {
        i.next();
        XInfoDB::BREAKPOINT breakPoint=i.value();

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
bool XInfoDB::breakpointToggle(quint64 nAddress)
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
    g_mapThreadInfos.insert(pThreadInfo->nThreadID,*pThreadInfo);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::removeThreadInfo(THREAD_INFO *pThreadInfo)
{
    g_mapThreadInfos.remove(pThreadInfo->nThreadID);
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
    for(QMap<quint64,XInfoDB::BREAKPOINT>::iterator it=getSoftwareBreakpoints()->begin();it!=getSoftwareBreakpoints()->end();)
    {
        if(it.value().sInfo==sFunctionName)
        {
            it=getSoftwareBreakpoints()->erase(it);
        }
        else
        {
            ++it;
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
QMap<qint64, XInfoDB::SHAREDOBJECT_INFO> *XInfoDB::getSharedObjectInfos()
{
    return &g_mapSharedObjectInfos;
}
#endif
#ifdef USE_XPROCESS
QMap<qint64, XInfoDB::THREAD_INFO> *XInfoDB::getThreadInfos()
{
    return &g_mapThreadInfos;
}
#endif
#ifdef USE_XPROCESS
QMap<QString, XInfoDB::FUNCTIONHOOK_INFO> *XInfoDB::getFunctionHookInfos()
{
    return &g_mapFunctionHookInfos;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::SHAREDOBJECT_INFO XInfoDB::findSharedInfoByName(QString sName)
{
    XInfoDB::SHAREDOBJECT_INFO result={};

    for(QMap<qint64,XInfoDB::SHAREDOBJECT_INFO>::iterator it=g_mapSharedObjectInfos.begin();it!=g_mapSharedObjectInfos.end();)
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

    for(QMap<qint64,XInfoDB::SHAREDOBJECT_INFO>::iterator it=g_mapSharedObjectInfos.begin();it!=g_mapSharedObjectInfos.end();)
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
XInfoDB::THREAD_INFO XInfoDB::findThreadInfoByID(qint64 nThreadID)
{
    XInfoDB::THREAD_INFO result={};

    for(QMap<qint64,XInfoDB::THREAD_INFO>::iterator it=g_mapThreadInfos.begin();it!=g_mapThreadInfos.end();)
    {
        XInfoDB::THREAD_INFO record=it.value();

        if(record.nThreadID==nThreadID)
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
quint64 XInfoDB::getFunctionAddress(QString sFunctionName)
{
    Q_UNUSED(sFunctionName)
    // TODO
    return 0;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setProcessInfo(PROCESS_INFO processInfo)
{
    g_processInfo=processInfo;
    g_mode=MODE_PROCESS;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentThread(XProcess::HANDLEID hidThread)
{
    g_hidThread=hidThread;
    g_mode=MODE_PROCESS;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::PROCESS_INFO *XInfoDB::getProcessInfo()
{
    return &g_processInfo;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateRegs(XREG_OPTIONS regOptions)
{
    g_statusPrev.mapRegs=g_statusCurrent.mapRegs; // TODO save nThreadID

    g_statusCurrent.mapRegs.clear();

#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_ALL; // All registers TODO Check regOptions | CONTEXT_FLOATING_POINT | CONTEXT_EXTENDED_REGISTERS;

    if(GetThreadContext(g_hidThread.hHandle,&context))
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
            g_statusCurrent.mapRegs.insert(XREG_EFLAGS,XBinary::getXVariant((quint32)(context.EFlags)));
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
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[0].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[0].High);
//                mapResult.insert("ST0",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[1].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[1].High);
//                mapResult.insert("ST1",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[2].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[2].High);
//                mapResult.insert("ST2",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[3].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[3].High);
//                mapResult.insert("ST3",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[4].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[4].High);
//                mapResult.insert("ST4",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[5].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[5].High);
//                mapResult.insert("ST5",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[6].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[6].High);
//                mapResult.insert("ST6",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.FloatRegisters[7].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.FloatRegisters[7].High);
//                mapResult.insert("ST7",xVariant);
        #endif
        }

        if(regOptions.bXMM)
        {
//                xVariant={};
//                xVariant.mode=XBinary::MODE_128;
//            #if defined(Q_PROCESSOR_X86_64)
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[0].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[0].High);
//                mapResult.insert("XMM0",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[1].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[1].High);
//                mapResult.insert("XMM1",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[2].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[2].High);
//                mapResult.insert("XMM2",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[3].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[3].High);
//                mapResult.insert("XMM3",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[4].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[4].High);
//                mapResult.insert("XMM4",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[5].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[5].High);
//                mapResult.insert("XMM5",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[6].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[6].High);
//                mapResult.insert("XMM6",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[7].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[7].High);
//                mapResult.insert("XMM7",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[8].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[8].High);
//                mapResult.insert("XMM8",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[9].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[9].High);
//                mapResult.insert("XMM9",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[10].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[10].High);
//                mapResult.insert("XMM10",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[11].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[11].High);
//                mapResult.insert("XMM11",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[12].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[12].High);
//                mapResult.insert("XMM12",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[13].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[13].High);
//                mapResult.insert("XMM13",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[14].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[14].High);
//                mapResult.insert("XMM14",xVariant);
//                xVariant.var.v_uint128.low=(quint64)(context.FltSave.XmmRegisters[15].Low);
//                xVariant.var.v_uint128.high=(quint64)(context.FltSave.XmmRegisters[15].High);
//                mapResult.insert("XMM15",xVariant);
//            #endif
//            mapResult.insert("MxCsr",(quint32)(context.MxCsr));
        }

    #ifdef QT_DEBUG
//        qDebug("DebugControl %s",XBinary::valueToHex((quint64)(context.DebugControl)).toLatin1().data());
//        qDebug("LastBranchToRip %s",XBinary::valueToHex((quint64)(context.LastBranchToRip)).toLatin1().data());
//        qDebug("LastBranchFromRip %s",XBinary::valueToHex((quint64)(context.LastBranchFromRip)).toLatin1().data());
//        qDebug("LastExceptionToRip %s",XBinary::valueToHex((quint64)(context.LastExceptionToRip)).toLatin1().data());
//        qDebug("LastExceptionFromRip %s",XBinary::valueToHex((quint64)(context.LastExceptionFromRip)).toLatin1().data());
     #if defined(Q_PROCESSOR_X86_64)
        qDebug("P1Home %s",XBinary::valueToHex((quint64)(context.P1Home)).toLatin1().data());
        qDebug("P2Home %s",XBinary::valueToHex((quint64)(context.P2Home)).toLatin1().data());
        qDebug("P3Home %s",XBinary::valueToHex((quint64)(context.P3Home)).toLatin1().data());
        qDebug("P4Home %s",XBinary::valueToHex((quint64)(context.P4Home)).toLatin1().data());
        qDebug("P5Home %s",XBinary::valueToHex((quint64)(context.P5Home)).toLatin1().data());
        qDebug("P6Home %s",XBinary::valueToHex((quint64)(context.P6Home)).toLatin1().data());
        qDebug("ContextFlags %s",XBinary::valueToHex((quint32)(context.ContextFlags)).toLatin1().data());
        qDebug("MxCsr %s",XBinary::valueToHex((quint32)(context.MxCsr)).toLatin1().data());
     #endif
    #endif
    }
#endif
#ifdef Q_OS_LINUX
    user_regs_struct regs={};
//    user_regs_struct regs;
    errno=0;

    if(ptrace(PTRACE_GETREGS,g_hidThread.nID,nullptr,&regs)!=-1)
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
            g_statusCurrent.mapRegs.insert(XREG_EFLAGS,XBinary::getXVariant((quint32)(regs.eflags)));
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
void XInfoDB::updateMemoryRegionsList()
{
    g_statusPrev.listMemoryRegions=g_statusCurrent.listMemoryRegions;

    g_statusCurrent.listMemoryRegions.clear();

    XProcess::HANDLEID hidProcess={};
    hidProcess.hHandle=g_processInfo.hProcessMemoryQuery;
    hidProcess.nID=g_processInfo.nProcessID;

    g_statusCurrent.listMemoryRegions=XProcess::getMemoryRegionsList(hidProcess,0,0xFFFFFFFFFFFFFFFF);
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
XBinary::XVARIANT XInfoDB::getCurrentReg(XREG reg)
{
    return _getReg(&(g_statusCurrent.mapRegs),reg);
}

bool XInfoDB::setCurrentReg(XREG reg, XBinary::XVARIANT variant)
{
    bool bResult=false;

    // TODO

    return bResult;
}
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
        if(!g_mapSoftwareBreakpoints.contains(nAddress))
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
                    g_mapSoftwareBreakpoints.insert(nAddress,bp);

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
bool XInfoDB::removeBreakPoint(quint64 nAddress, BPT bpType)
{
    bool bResult=false;

    if(bpType==BPT_CODE_SOFTWARE)
    {
        if(g_mapSoftwareBreakpoints.contains(nAddress))
        {
            BREAKPOINT bp=g_mapSoftwareBreakpoints.value(nAddress);

            if(write_array(nAddress,(char *)bp.origData,bp.nOrigDataSize)) // TODO Check
            {
                g_mapSoftwareBreakpoints.remove(nAddress);

                bResult=true;
            }
        }
    }
    else if(bpType==XInfoDB::BPT_CODE_HARDWARE)
    {
        // TODO
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::isBreakPointPresent(quint64 nAddress,BPT bpType)
{
    bool bResult=false;

    if(bpType==BPT_CODE_SOFTWARE)
    {
        bResult=g_mapSoftwareBreakpoints.contains(nAddress);
    }
    else if(bpType==XInfoDB::BPT_CODE_HARDWARE)
    {
        bResult=g_mapHardwareBreakpoints.contains(nAddress);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
QMap<quint64,XInfoDB::BREAKPOINT> *XInfoDB::getSoftwareBreakpoints()
{
    return &g_mapSoftwareBreakpoints;
}
#endif
#ifdef USE_XPROCESS
QMap<quint64, XInfoDB::BREAKPOINT> *XInfoDB::getHardwareBreakpoints()
{
    return &g_mapHardwareBreakpoints;
}
#endif
#ifdef USE_XPROCESS
QMap<qint64, XInfoDB::BREAKPOINT> *XInfoDB::getThreadBreakpoints()
{
    return &g_mapThreadBreakpoints;
}
#endif
bool XInfoDB::isRegChanged(XREG reg)
{
    return !(XBinary::isXVariantEqual(_getReg(&(g_statusCurrent.mapRegs),reg),_getReg(&(g_statusPrev.mapRegs),reg)));
}

QList<XBinary::MEMORY_REPLACE> XInfoDB::getMemoryReplaces(quint64 nBase, quint64 nSize)
{
    QList<XBinary::MEMORY_REPLACE> listResult;
#ifdef USE_XPROCESS
    QMapIterator<quint64,XInfoDB::BREAKPOINT> i(g_mapSoftwareBreakpoints);

    while (i.hasNext())
    {
        i.next();
        XInfoDB::BREAKPOINT breakPoint=i.value();

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

QString XInfoDB::regIdToString(XREG reg)
{
    QString sResult="Unknown";

    if      (reg==XREG_NONE)        sResult=QString("");
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

    return sResult;
}

XInfoDB::XREG XInfoDB::getSubReg32(XREG reg)
{
    XREG result=XREG_NONE;

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

    return result;
}

XInfoDB::XREG XInfoDB::getSubReg16(XREG reg)
{
    XREG result=XREG_NONE;

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

    return result;
}

XInfoDB::XREG XInfoDB::getSubReg8H(XREG reg)
{
    XREG result=XREG_NONE;

    if      ((reg==XREG_RAX)||(reg==XREG_EAX)||(reg==XREG_AX))      result=XREG_AH;
    else if ((reg==XREG_RCX)||(reg==XREG_ECX)||(reg==XREG_CX))      result=XREG_CH;
    else if ((reg==XREG_RDX)||(reg==XREG_EDX)||(reg==XREG_DX))      result=XREG_DH;
    else if ((reg==XREG_RBX)||(reg==XREG_EBX)||(reg==XREG_BX))      result=XREG_BH;

    return result;
}

XInfoDB::XREG XInfoDB::getSubReg8L(XREG reg)
{
    XREG result=XREG_NONE;

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

    return result;
}

XInfoDB::RECORD_INFO XInfoDB::getRecordInfo(quint64 nValue,RI_TYPE riType)
{
    RECORD_INFO result={};

    result.nAddress=-1;

    if((nValue>=g_processInfo.nImageBase)&&(nValue<(g_processInfo.nImageBase+g_processInfo.nImageSize)))
    {
        result.sModule=g_processInfo.sBaseFileName;
        result.nAddress=nValue;
    }
    else
    {
        SHAREDOBJECT_INFO sbi=findSharedInfoByAddress(nValue);

        if(sbi.nImageSize!=0)
        {
            result.sModule=sbi.sName;
            result.nAddress=nValue;
        }
    }

    if( (riType==RI_TYPE_GENERAL)||
        (riType==RI_TYPE_DATA)||
        (riType==RI_TYPE_ANSI)||
        (riType==RI_TYPE_UNICODE)||
        (riType==RI_TYPE_UTF8))
    {
        if(result.nAddress!=(quint64)-1)
        {
            result.baData=read_array(result.nAddress,32);
        }
    }

    if( (riType==RI_TYPE_GENERAL)||
        (riType==RI_TYPE_SYMBOL))
    {
        if(result.nAddress!=(quint64)-1)
        {
            // TODO getSymbol
            // TODO
            // If not use address
            if(riType==RI_TYPE_SYMBOL)
            {
                if(result.sSymbol=="")
                {
                    result.sSymbol=QString("%1.%2").arg(result.sModule,XBinary::valueToHexOS(result.nAddress));
                }
            }
        }
    }

    return result;
}

QString XInfoDB::recordInfoToString(RECORD_INFO recordInfo,RI_TYPE riType)
{
    QString sResult;

    if(recordInfo.nAddress!=(quint64)-1)
    {
        if(riType==RI_TYPE_GENERAL)
        {
            QString sAnsiString;
            QString sUnicodeString;

            if(XBinary::_read_uint8(recordInfo.baData.data())<128)
            {
                sAnsiString=QString::fromLatin1(recordInfo.baData);
            }

            if(XBinary::_read_uint16(recordInfo.baData.data())<128)
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
                sResult=QString("%1.%2").arg(recordInfo.sModule,XBinary::valueToHexOS(recordInfo.nAddress));
            }
        }
        else if(riType==RI_TYPE_ADDRESS)
        {
            sResult=QString("%1.%2").arg(recordInfo.sModule,XBinary::valueToHexOS(recordInfo.nAddress));
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
    }

    return sResult;
}

XBinary::XVARIANT XInfoDB::_getReg(QMap<XREG,XBinary::XVARIANT> *pMapRegs,XREG reg)
{
    // TODO AX AL AH
    XBinary::XVARIANT result={};

    XREG _reg=reg;

    if( (reg==XREG_CF)||(reg==XREG_PF)||(reg==XREG_AF)||
        (reg==XREG_ZF)||(reg==XREG_SF)||(reg==XREG_TF)||
        (reg==XREG_IF)||(reg==XREG_DF)||(reg==XREG_OF))
    {
        _reg=XREG_EFLAGS;
    }
#ifdef Q_PROCESSOR_X86_32
    // TODO
#endif
#ifdef Q_PROCESSOR_X86_64
    // TODO
#endif

    result=pMapRegs->value(_reg);

    if(result.mode!=XBinary::MODE_UNKNOWN)
    {
        if      (reg==XREG_CF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0001));
        else if (reg==XREG_PF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0004));
        else if (reg==XREG_AF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0010));
        else if (reg==XREG_ZF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0040));
        else if (reg==XREG_SF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0080));
        else if (reg==XREG_TF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0100));
        else if (reg==XREG_IF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0200));
        else if (reg==XREG_DF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0400));
        else if (reg==XREG_OF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0800));
    }

    return result;
}
