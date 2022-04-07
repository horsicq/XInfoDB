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
    g_hidProcess={};
}

XInfoDB::~XInfoDB()
{
}

void XInfoDB::setProcess(XProcess::HANDLEID hidProcess)
{
    g_hidProcess=hidProcess;
    g_mode=MODE_PROCESS;
}

void XInfoDB::updateRegs(XProcess::HANDLEID hidThread, REG_OPTIONS regOptions)
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
            g_statusCurrent.mapRegs.insert(REG_RAX,XBinary::getXVariant((quint64)(context.Rax)));
            g_statusCurrent.mapRegs.insert(REG_RBX,XBinary::getXVariant((quint64)(context.Rbx)));
            g_statusCurrent.mapRegs.insert(REG_RCX,XBinary::getXVariant((quint64)(context.Rcx)));
            g_statusCurrent.mapRegs.insert(REG_RDX,XBinary::getXVariant((quint64)(context.Rdx)));
            g_statusCurrent.mapRegs.insert(REG_RBP,XBinary::getXVariant((quint64)(context.Rbp)));
            g_statusCurrent.mapRegs.insert(REG_RSP,XBinary::getXVariant((quint64)(context.Rsp)));
            g_statusCurrent.mapRegs.insert(REG_RSI,XBinary::getXVariant((quint64)(context.Rsi)));
            g_statusCurrent.mapRegs.insert(REG_RDI,XBinary::getXVariant((quint64)(context.Rdi)));
            g_statusCurrent.mapRegs.insert(REG_R8,XBinary::getXVariant((quint64)(context.R8)));
            g_statusCurrent.mapRegs.insert(REG_R9,XBinary::getXVariant((quint64)(context.R9)));
            g_statusCurrent.mapRegs.insert(REG_R10,XBinary::getXVariant((quint64)(context.R10)));
            g_statusCurrent.mapRegs.insert(REG_R11,XBinary::getXVariant((quint64)(context.R11)));
            g_statusCurrent.mapRegs.insert(REG_R12,XBinary::getXVariant((quint64)(context.R12)));
            g_statusCurrent.mapRegs.insert(REG_R13,XBinary::getXVariant((quint64)(context.R13)));
            g_statusCurrent.mapRegs.insert(REG_R14,XBinary::getXVariant((quint64)(context.R14)));
            g_statusCurrent.mapRegs.insert(REG_R15,XBinary::getXVariant((quint64)(context.R15)));
        #endif
        }

        if(regOptions.bIP)
        {
        #ifdef Q_PROCESSOR_X86_32
            result.EIP=(quint32)(context.Eip);
        #endif
        #ifdef Q_PROCESSOR_X86_64
            g_statusCurrent.mapRegs.insert(REG_RIP,XBinary::getXVariant((quint64)(context.Rip)));
        #endif
        }

        if(regOptions.bFlags)
        {
            g_statusCurrent.mapRegs.insert(REG_EFLAGS,XBinary::getXVariant((quint32)(context.EFlags)));
        }

        if(regOptions.bSegments)
        {
            g_statusCurrent.mapRegs.insert(REG_CS,XBinary::getXVariant((quint16)(context.SegCs)));
            g_statusCurrent.mapRegs.insert(REG_FS,XBinary::getXVariant((quint16)(context.SegFs)));
            g_statusCurrent.mapRegs.insert(REG_ES,XBinary::getXVariant((quint16)(context.SegEs)));
            g_statusCurrent.mapRegs.insert(REG_DS,XBinary::getXVariant((quint16)(context.SegDs)));
            g_statusCurrent.mapRegs.insert(REG_GS,XBinary::getXVariant((quint16)(context.SegGs)));
            g_statusCurrent.mapRegs.insert(REG_SS,XBinary::getXVariant((quint16)(context.SegSs)));
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
            g_statusCurrent.mapRegs.insert(REG_DR0,XBinary::getXVariant((quint64)(context.Dr0)));
            g_statusCurrent.mapRegs.insert(REG_DR1,XBinary::getXVariant((quint64)(context.Dr1)));
            g_statusCurrent.mapRegs.insert(REG_DR2,XBinary::getXVariant((quint64)(context.Dr2)));
            g_statusCurrent.mapRegs.insert(REG_DR3,XBinary::getXVariant((quint64)(context.Dr3)));
            g_statusCurrent.mapRegs.insert(REG_DR6,XBinary::getXVariant((quint64)(context.Dr6)));
            g_statusCurrent.mapRegs.insert(REG_DR7,XBinary::getXVariant((quint64)(context.Dr7)));
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
}

void XInfoDB::updateMemoryRegionsList()
{
    g_statusPrev.listMemoryRegions=g_statusCurrent.listMemoryRegions;

    g_statusCurrent.listMemoryRegions.clear();

    g_statusCurrent.listMemoryRegions=XProcess::getMemoryRegionsList(g_hidProcess,0,0xFFFFFFFFFFFFFFFF);

}

void XInfoDB::updateModulesList()
{
    g_statusPrev.listModules=g_statusCurrent.listModules;

    g_statusCurrent.listModules.clear();

    g_statusCurrent.listModules=XProcess::getModulesList(g_hidProcess.nID);
}

XBinary::XVARIANT XInfoDB::getCurrentReg(REG reg)
{
    return _getReg(&(g_statusCurrent.mapRegs),reg);
}

QList<XBinary::MEMORY_REGION> *XInfoDB::getCurrentMemoryRegionsList()
{
    return &(g_statusCurrent.listMemoryRegions);
}

QList<XBinary::MODULE> *XInfoDB::getCurrentModulesList()
{
    return &(g_statusCurrent.listModules);
}

bool XInfoDB::isRegChanged(REG reg)
{
    // TODO
    return false;
}

QString XInfoDB::regIdToString(REG reg)
{
    QString result="DDD";

    return result;
}

XBinary::XVARIANT XInfoDB::_getReg(QMap<REG, XBinary::XVARIANT> *pMapRegs, REG reg)
{
    XBinary::XVARIANT result={};

    if( (reg==REG_CF)||(reg==REG_PF)||(reg==REG_AF)||
        (reg==REG_ZF)||(reg==REG_SF)||(reg==REG_TF)||
        (reg==REG_IF)||(reg==REG_DF)||(reg==REG_OF))
    {
        reg=REG_EFLAGS;
    }

    result=pMapRegs->value(reg);

    if(result.mode!=XBinary::MODE_UNKNOWN)
    {
        if      (reg==REG_CF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0001));
        else if (reg==REG_PF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0004));
        else if (reg==REG_AF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0010));
        else if (reg==REG_ZF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0040));
        else if (reg==REG_SF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0080));
        else if (reg==REG_TF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0100));
        else if (reg==REG_IF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0200));
        else if (reg==REG_DF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0400));
        else if (reg==REG_OF) result=XBinary::getXVariant(bool((result.var.v_uint32)&0x0800));
    }

    return result;
}
