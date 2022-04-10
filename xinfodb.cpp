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
#endif
}

XInfoDB::~XInfoDB()
{
}

quint32 XInfoDB::read_uint32(quint64 nAddress, bool bIsBigEndian)
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
    sResult=XProcess::read_ansiString(g_processInfo.hProcessMemoryIO,nAddress,nMaxSize);
#endif
    return sResult;
}

#ifdef USE_XPROCESS
void XInfoDB::setProcessInfo(PROCESS_INFO processInfo)
{
    g_processInfo=processInfo;
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
void XInfoDB::updateRegs(XProcess::HANDLEID hidThread, XREG_OPTIONS regOptions)
{
    g_statusPrev.mapRegs=g_statusCurrent.mapRegs; // TODO save nThreadID

    g_statusCurrent.mapRegs.clear();

#ifdef Q_OS_WIN
    CONTEXT context={0};
    context.ContextFlags=CONTEXT_ALL; // All registers TODO Check regOptions | CONTEXT_FLOATING_POINT | CONTEXT_EXTENDED_REGISTERS;

    if(GetThreadContext(hidThread.hHandle,&context))
    {
        if(regOptions.bGeneral)
        {
        #ifdef Q_PROCESSOR_X86_32
            result.EAX=(quint32)(context.Eax);
            result.EBX=(quint32)(context.Ebx);
            result.ECX=(quint32)(context.Ecx);
            result.EDX=(quint32)(context.Edx);
            result.EBP=(quint32)(context.Ebp);
            result.ESP=(quint32)(context.Esp);
            result.ESI=(quint32)(context.Esi);
            result.EDI=(quint32)(context.Edi);
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
            result.EIP=(quint32)(context.Eip);
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
            result.DR[0]=(quint32)(context.Dr0);
            result.DR[1]=(quint32)(context.Dr1);
            result.DR[2]=(quint32)(context.Dr2);
            result.DR[3]=(quint32)(context.Dr3);
            result.DR[6]=(quint32)(context.Dr6);
            result.DR[7]=(quint32)(context.Dr7);
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

    if(ptrace(PTRACE_GETREGS,hidThread.nID,nullptr,&regs)!=-1)
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
    hidProcess.hHandle=g_processInfo.hProcessMemoryIO;
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
#ifdef USE_XPROCESS
QList<XBinary::MEMORY_REGION> *XInfoDB::getCurrentMemoryRegionsList()
{
    return &(g_statusCurrent.listMemoryRegions);
}
#endif
#ifdef USE_XPROCESS
QList<XBinary::MODULE> *XInfoDB::getCurrentModulesList()
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
bool XInfoDB::isBreakPointPresent(quint64 nAddress, BPT bpType)
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
QMap<qint64, XInfoDB::BREAKPOINT> *XInfoDB::getSoftwareBreakpoints()
{
    return &g_mapSoftwareBreakpoints;
}
#endif
#ifdef USE_XPROCESS
QMap<qint64, XInfoDB::BREAKPOINT> *XInfoDB::getHardwareBreakpoints()
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

QString XInfoDB::regIdToString(XREG reg)
{
    QString sResult="Unknown";

    if      (reg==XREG_EAX)      sResult=QString("EAX");
    else if (reg==XREG_ECX)      sResult=QString("ECX");
    else if (reg==XREG_EDX)      sResult=QString("EDX");
    else if (reg==XREG_EBX)      sResult=QString("EBX");
    else if (reg==XREG_ESP)      sResult=QString("ESP");
    else if (reg==XREG_EBP)      sResult=QString("EBP");
    else if (reg==XREG_ESI)      sResult=QString("ESI");
    else if (reg==XREG_EDI)      sResult=QString("EDI");
    else if (reg==XREG_EIP)      sResult=QString("EIP");
    else if (reg==XREG_RAX)      sResult=QString("RAX");
    else if (reg==XREG_RCX)      sResult=QString("RCX");
    else if (reg==XREG_RDX)      sResult=QString("RDX");
    else if (reg==XREG_RBX)      sResult=QString("RBX");
    else if (reg==XREG_RSP)      sResult=QString("RSP");
    else if (reg==XREG_RBP)      sResult=QString("RBP");
    else if (reg==XREG_RSI)      sResult=QString("RSI");
    else if (reg==XREG_RDI)      sResult=QString("RDI");
    else if (reg==XREG_R8)       sResult=QString("R8");
    else if (reg==XREG_R9)       sResult=QString("R9");
    else if (reg==XREG_R10)      sResult=QString("R10");
    else if (reg==XREG_R11)      sResult=QString("R11");
    else if (reg==XREG_R12)      sResult=QString("R12");
    else if (reg==XREG_R13)      sResult=QString("R13");
    else if (reg==XREG_R14)      sResult=QString("R14");
    else if (reg==XREG_R15)      sResult=QString("R15");
    else if (reg==XREG_RIP)      sResult=QString("RIP");
    else if (reg==XREG_EFLAGS)   sResult=QString("EFLAGS");
    else if (reg==XREG_CS)       sResult=QString("CS");
    else if (reg==XREG_DS)       sResult=QString("DS");
    else if (reg==XREG_ES)       sResult=QString("ES");
    else if (reg==XREG_FS)       sResult=QString("FS");
    else if (reg==XREG_GS)       sResult=QString("GS");
    else if (reg==XREG_SS)       sResult=QString("SS");
    else if (reg==XREG_DR0)      sResult=QString("DR0");
    else if (reg==XREG_DR1)      sResult=QString("DR1");
    else if (reg==XREG_DR2)      sResult=QString("DR2");
    else if (reg==XREG_DR3)      sResult=QString("DR3");
    else if (reg==XREG_DR6)      sResult=QString("DR6");
    else if (reg==XREG_DR7)      sResult=QString("DR7");
    else if (reg==XREG_CF)       sResult=QString("CF");
    else if (reg==XREG_PF)       sResult=QString("PF");
    else if (reg==XREG_AF)       sResult=QString("AF");
    else if (reg==XREG_ZF)       sResult=QString("ZF");
    else if (reg==XREG_SF)       sResult=QString("SF");
    else if (reg==XREG_TF)       sResult=QString("TF");
    else if (reg==XREG_IF)       sResult=QString("IF");
    else if (reg==XREG_DF)       sResult=QString("DF");
    else if (reg==XREG_OF)       sResult=QString("OF");
    else if (reg==XREG_ST0)      sResult=QString("ST0");
    else if (reg==XREG_ST1)      sResult=QString("ST1");
    else if (reg==XREG_ST2)      sResult=QString("ST2");
    else if (reg==XREG_ST3)      sResult=QString("ST3");
    else if (reg==XREG_ST4)      sResult=QString("ST4");
    else if (reg==XREG_ST5)      sResult=QString("ST5");
    else if (reg==XREG_ST6)      sResult=QString("ST6");
    else if (reg==XREG_ST7)      sResult=QString("ST7");
    else if (reg==XREG_XMM0)     sResult=QString("XMM0");
    else if (reg==XREG_XMM1)     sResult=QString("XMM1");
    else if (reg==XREG_XMM2)     sResult=QString("XMM2");
    else if (reg==XREG_XMM3)     sResult=QString("XMM3");
    else if (reg==XREG_XMM4)     sResult=QString("XMM4");
    else if (reg==XREG_XMM5)     sResult=QString("XMM5");
    else if (reg==XREG_XMM6)     sResult=QString("XMM6");
    else if (reg==XREG_XMM7)     sResult=QString("XMM7");
    else if (reg==XREG_XMM8)     sResult=QString("XMM8");
    else if (reg==XREG_XMM9)     sResult=QString("XMM9");
    else if (reg==XREG_XMM10)    sResult=QString("XMM10");
    else if (reg==XREG_XMM11)    sResult=QString("XMM11");
    else if (reg==XREG_XMM12)    sResult=QString("XMM12");
    else if (reg==XREG_XMM13)    sResult=QString("XMM13");
    else if (reg==XREG_XMM14)    sResult=QString("XMM14");
    else if (reg==XREG_XMM15)    sResult=QString("XMM15");

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
