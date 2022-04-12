/* Copyright (c) 2022 hors<horsicq@gmail.com>
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
#ifndef XINFODB_H
#define XINFODB_H

#include <QObject>
#include "xbinary.h"
#ifdef USE_XPROCESS
#include "xprocess.h"
#endif

class XInfoDB : public QObject
{
    Q_OBJECT
public:

    enum MODE
    {
        MODE_UNKNOWN=0,
    #ifdef USE_XPROCESS
        MODE_PROCESS
    #endif
    };

    struct XREG_OPTIONS
    {
        bool bGeneral;
        bool bIP;
        bool bFlags;
        bool bSegments;
        bool bDebug;
        bool bFloat;
        bool bXMM;
    };

    enum XREG
    {
        XREG_UNKNOWN=0,
        XREG_EAX,
        XREG_ECX,
        XREG_EDX,
        XREG_EBX,
        XREG_ESP,
        XREG_EBP,
        XREG_ESI,
        XREG_EDI,
        XREG_EIP,
        XREG_RAX,
        XREG_RCX,
        XREG_RDX,
        XREG_RBX,
        XREG_RSP,
        XREG_RBP,
        XREG_RSI,
        XREG_RDI,
        XREG_R8,
        XREG_R9,
        XREG_R10,
        XREG_R11,
        XREG_R12,
        XREG_R13,
        XREG_R14,
        XREG_R15,
        XREG_RIP,
        XREG_EFLAGS,
        XREG_CS,
        XREG_DS,
        XREG_ES,
        XREG_FS,
        XREG_GS,
        XREG_SS,
        XREG_DR0,
        XREG_DR1,
        XREG_DR2,
        XREG_DR3,
        XREG_DR6,
        XREG_DR7,
        XREG_CF,
        XREG_PF,
        XREG_AF,
        XREG_ZF,
        XREG_SF,
        XREG_TF,
        XREG_IF,
        XREG_DF,
        XREG_OF,
        XREG_ST0,
        XREG_ST1,
        XREG_ST2,
        XREG_ST3,
        XREG_ST4,
        XREG_ST5,
        XREG_ST6,
        XREG_ST7,
        XREG_XMM0,
        XREG_XMM1,
        XREG_XMM2,
        XREG_XMM3,
        XREG_XMM4,
        XREG_XMM5,
        XREG_XMM6,
        XREG_XMM7,
        XREG_XMM8,
        XREG_XMM9,
        XREG_XMM10,
        XREG_XMM11,
        XREG_XMM12,
        XREG_XMM13,
        XREG_XMM14,
        XREG_XMM15,
    };

    enum BPT
    {
        BPT_UNKNOWN=0,
        BPT_CODE_SOFTWARE,    // for X86 0xCC Check for ARM
        BPT_CODE_HARDWARE
    };

    enum BPI
    {
        BPI_UNKNOWN=0,
        BPI_USER,
        BPI_PROCESSENTRYPOINT,
        BPI_PROGRAMENTRYPOINT,
        BPI_TLSFUNCTION, // TODO
        BPI_FUNCTIONENTER,
        BPI_FUNCTIONLEAVE,
        BPI_STEP,
        BPI_STEPINTO,
        BPI_STEPOVER
    };

    struct BREAKPOINT
    {
        quint64 nAddress;
        qint64 nSize;
        qint32 nCount;
        BPT bpType;
        BPI bpInfo;
        QString sInfo;
        qint32 nOrigDataSize;
        char origData[4]; // TODO consts check
        QString sGUID;
    };

    struct THREAD_INFO
    {
        qint64 nThreadID;
        qint64 nThreadLocalBase;
        quint64 nStartAddress;
        void *hThread;
    };

    struct EXITTHREAD_INFO
    {
        qint64 nThreadID;
        qint64 nExitCode;
    };

    struct PROCESS_INFO
    {
        qint64 nProcessID;
        qint64 nThreadID;
        QString sFileName;
        quint64 nImageBase;
        quint64 nImageSize;
        quint64 nStartAddress;
        quint64 nThreadLocalBase;
        void *hProcessMemoryIO;
        void *hProcessMemoryQuery;
        void *hMainThread;
    };

    struct EXITPROCESS_INFO
    {
        qint64 nProcessID;
        qint64 nThreadID;
        qint64 nExitCode;
    };

    struct SHAREDOBJECT_INFO // DLL on Windows
    {
        QString sName;
        QString sFileName;
        quint64 nImageBase;
        quint64 nImageSize;
    };

    struct DEBUGSTRING_INFO
    {
        qint64 nThreadID;
        QString sDebugString;
    };

    struct BREAKPOINT_INFO
    {
        quint64 nAddress;
        XInfoDB::BPT bpType;
        XInfoDB::BPI bpInfo;
        QString sInfo;
        qint64 nProcessID;
        void *pHProcessMemoryIO;
        void *pHProcessMemoryQuery;
        qint64 nThreadID;
        void *pHThread;
    };

    struct PROCESSENTRY_INFO
    {
        quint64 nAddress;
    };

    struct FUNCTIONHOOK_INFO
    {
        QString sName;
        quint64 nAddress;
    };

    struct FUNCTION_INFO
    {
        QString sName;
        quint64 nAddress;
        qint64 nRetAddress;
        quint64 nParameter0;
        quint64 nParameter1;
        quint64 nParameter2;
        quint64 nParameter3;
        quint64 nParameter4;
        quint64 nParameter5;
        quint64 nParameter6;
        quint64 nParameter7;
        quint64 nParameter8;
        quint64 nParameter9;
    };

    explicit XInfoDB(QObject *pParent=nullptr);
    ~XInfoDB();

    void reload(bool bDataReload);

    quint32 read_uint32(quint64 nAddress,bool bIsBigEndian=false);
    quint64 read_uint64(quint64 nAddress,bool bIsBigEndian=false);
    qint64 read_array(quint64 nAddress,char *pData,quint64 nSize);
    qint64 write_array(quint64 nAddress,char *pData,quint64 nSize);
    QByteArray read_array(quint64 nAddress,quint64 nSize);
    QString read_ansiString(quint64 nAddress,quint64 nMaxSize=256);
    QString read_unicodeString(quint64 nAddress,quint64 nMaxSize=256); // TODO endian ??
#ifdef USE_XPROCESS
    void setProcessInfo(PROCESS_INFO processInfo);
    PROCESS_INFO *getProcessInfo();
    void updateRegs(XProcess::HANDLEID hidThread,XREG_OPTIONS regOptions);
    void updateMemoryRegionsList();
    void updateModulesList();
    QList<XProcess::MEMORY_REGION> *getCurrentMemoryRegionsList();
    QList<XProcess::MODULE> *getCurrentModulesList();
    bool addBreakPoint(quint64 nAddress,BPT bpType=BPT_CODE_SOFTWARE,BPI bpInfo=BPI_UNKNOWN,qint32 nCount=-1,QString sInfo=QString(),QString sGUID=QString());
    bool removeBreakPoint(quint64 nAddress,BPT bpType=BPT_CODE_SOFTWARE);
    bool isBreakPointPresent(quint64 nAddress,BPT bpType=BPT_CODE_SOFTWARE);
    QMap<qint64,BREAKPOINT> *getSoftwareBreakpoints();
    QMap<qint64,BREAKPOINT> *getHardwareBreakpoints();
    QMap<qint64,BREAKPOINT> *getThreadBreakpoints();
    bool breakpointToggle(quint64 nAddress);
    QList<XBinary::MEMORY_REPLACE> getMemoryReplaces(quint64 nBase=0,quint64 nSize=0xFFFFFFFFFFFFFFFF);

    void addSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void removeSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);

    void addThreadInfo(XInfoDB::THREAD_INFO *pThreadInfo);
    void removeThreadInfo(XInfoDB::THREAD_INFO *pThreadInfo);

    bool setFunctionHook(QString sFunctionName);
    bool removeFunctionHook(QString sFunctionName);

    QMap<qint64,SHAREDOBJECT_INFO> *getSharedObjectInfos();
    QMap<qint64,THREAD_INFO> *getThreadInfos();
    QMap<QString,FUNCTIONHOOK_INFO> *getFunctionHookInfos();

    SHAREDOBJECT_INFO findSharedInfoByName(QString sName);
    SHAREDOBJECT_INFO findSharedInfoByAddress(quint64 nAddress);

    quint64 getFunctionAddress(QString sFunctionName);
#endif
    XBinary::XVARIANT getCurrentReg(XREG reg);
    bool isRegChanged(XREG reg);

    static QString regIdToString(XREG reg);

signals:
    void dataChanged(bool bDataReload);

private:

    struct STATUS
    {
        QMap<XREG,XBinary::XVARIANT> mapRegs;
    #ifdef USE_XPROCESS
        QList<XProcess::MEMORY_REGION> listMemoryRegions;
        QList<XProcess::MODULE> listModules;
    #endif
    };
    XBinary::XVARIANT _getReg(QMap<XREG,XBinary::XVARIANT> *pMapRegs,XREG reg);
private:
#ifdef USE_XPROCESS
    XInfoDB::PROCESS_INFO g_processInfo;
    QMap<qint64,BREAKPOINT> g_mapSoftwareBreakpoints;       // Address/BP
    QMap<qint64,BREAKPOINT> g_mapHardwareBreakpoints;       // Address/BP
    QMap<qint64,BREAKPOINT> g_mapThreadBreakpoints;         // STEPS, ThreadID/BP
    QMap<qint64,SHAREDOBJECT_INFO> g_mapSharedObjectInfos;
    QMap<qint64,THREAD_INFO> g_mapThreadInfos;
    QMap<QString,FUNCTIONHOOK_INFO> g_mapFunctionHookInfos;
#endif
    MODE g_mode;
    STATUS g_statusCurrent;
    STATUS g_statusPrev;
};

#endif // XINFODB_H
