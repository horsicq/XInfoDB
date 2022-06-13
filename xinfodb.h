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
#include <QMutex>
#include "xformats.h"
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
        MODE_DEVICE,
    #ifdef USE_XPROCESS
        MODE_PROCESS
    #endif
    };
#ifdef USE_XPROCESS
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
        XREG_NONE,
    #ifdef Q_PROCESSOR_X86
        XREG_AX,
        XREG_CX,
        XREG_DX,
        XREG_BX,
        XREG_SP,
        XREG_BP,
        XREG_SI,
        XREG_DI,
        XREG_IP,
        XREG_FLAGS,
        XREG_EAX,
        XREG_ECX,
        XREG_EDX,
        XREG_EBX,
        XREG_ESP,
        XREG_EBP,
        XREG_ESI,
        XREG_EDI,
        XREG_EIP,
        XREG_EFLAGS,
    #ifdef Q_PROCESSOR_X86_64
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
        XREG_RFLAGS,
    #endif
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
        XREG_AH,
        XREG_CH,
        XREG_DH,
        XREG_BH,
        XREG_AL,
        XREG_CL,
        XREG_DL,
        XREG_BL,
    #ifdef Q_PROCESSOR_X86_64
        XREG_SPL,
        XREG_BPL,
        XREG_SIL,
        XREG_DIL,
        XREG_R8D,
        XREG_R9D,
        XREG_R10D,
        XREG_R11D,
        XREG_R12D,
        XREG_R13D,
        XREG_R14D,
        XREG_R15D,
        XREG_R8W,
        XREG_R9W,
        XREG_R10W,
        XREG_R11W,
        XREG_R12W,
        XREG_R13W,
        XREG_R14W,
        XREG_R15W,
        XREG_R8B,
        XREG_R9B,
        XREG_R10B,
        XREG_R11B,
        XREG_R12B,
        XREG_R13B,
        XREG_R14B,
        XREG_R15B,
    #endif
    #endif
    };

    enum BPT
    {
        BPT_UNKNOWN=0,
        BPT_CODE_SOFTWARE,    // for X86 0xCC Check for ARM
        BPT_CODE_HARDWARE,
        BPT_CODE_MEMORY
    };

    enum BPI
    {
        BPI_UNKNOWN=0,
        BPI_SYSTEM,
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
        // TODO bValid
        XADDR nAddress;
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
        X_ID nThreadID;
        qint64 nThreadLocalBase;
        XADDR nStartAddress;
    #ifdef Q_OS_WIN
        X_HANDLE hThread;
    #endif
    };

    struct EXITTHREAD_INFO
    {
        qint64 nThreadID;
        qint64 nExitCode;
    };

    struct PROCESS_INFO
    {
        qint64 nProcessID;
        qint64 nMainThreadID; // TODO Check mb Remove
        QString sFileName;
        QString sBaseFileName;
        XADDR nImageBase;
        quint64 nImageSize;
        XADDR nStartAddress;
        XADDR nThreadLocalBase;
    #ifdef Q_OS_LINUX
        void *hProcessMemoryIO;
        void *hProcessMemoryQuery;
    #endif
    #ifdef Q_OS_WIN
        X_HANDLE hMainThread;
        X_HANDLE hProcess;
    #endif
    #ifdef Q_OS_MACOS
        X_HANDLE hProcess;
    #endif
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
        XADDR nImageBase;
        quint64 nImageSize;
    };

    struct DEBUGSTRING_INFO
    {
        qint64 nThreadID;
        QString sDebugString;
    };

    struct BREAKPOINT_INFO
    {
        XADDR nAddress;
        XInfoDB::BPT bpType;
        XInfoDB::BPI bpInfo;
        QString sInfo;
        X_ID nProcessID;
    #ifdef Q_OS_WIN
        X_HANDLE hProcess;
    #endif
    #ifdef Q_OS_LINUX
        X_HANDLE_IO pHProcessMemoryIO;      // TODO rename
        X_HANDLE_MQ pHProcessMemoryQuery;   // TODO rename
    #endif
    #ifdef Q_OS_MACOS
        X_HANDLE hProcess;
    #endif
        X_ID nThreadID;
    #ifdef Q_OS_WIN
        X_HANDLE hThread;
    #endif
    };

    struct PROCESSENTRY_INFO
    {
        XADDR nAddress;
    };

    struct FUNCTIONHOOK_INFO
    {
        QString sName;
        XADDR nAddress;
    };

    struct FUNCTION_INFO
    {
        QString sName;
        XADDR nAddress;
        XADDR nRetAddress;
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
#endif

    explicit XInfoDB(QObject *pParent=nullptr);
    ~XInfoDB();

    void setDevice(QIODevice *pDevice,XBinary::FT fileType=XBinary::FT_UNKNOWN);
    QIODevice *getDevice();
    XBinary::FT getFileType();

    void reload(bool bDataReload);

    quint32 read_uint32(XADDR nAddress,bool bIsBigEndian=false);
    quint64 read_uint64(XADDR nAddress,bool bIsBigEndian=false);
    qint64 read_array(XADDR nAddress,char *pData,quint64 nSize);
    qint64 write_array(XADDR nAddress,char *pData,quint64 nSize);
    QByteArray read_array(XADDR nAddress,quint64 nSize);
    QString read_ansiString(XADDR nAddress,quint64 nMaxSize=256);
    QString read_unicodeString(XADDR nAddress,quint64 nMaxSize=256); // TODO endian ??
    QString read_utf8String(XADDR nAddress,quint64 nMaxSize=256);
#ifdef USE_XPROCESS
    void setProcessInfo(PROCESS_INFO processInfo);
    PROCESS_INFO *getProcessInfo();
    void updateRegsById(X_ID nThreadId,XREG_OPTIONS regOptions);
    void updateRegsByHandle(X_HANDLE hThread,XREG_OPTIONS regOptions);
    void updateMemoryRegionsList();
    void updateModulesList();
    QList<XProcess::MEMORY_REGION> *getCurrentMemoryRegionsList();
    QList<XProcess::MODULE> *getCurrentModulesList();
    bool addBreakPoint(XADDR nAddress,BPT bpType=BPT_CODE_SOFTWARE,BPI bpInfo=BPI_UNKNOWN,qint32 nCount=-1,QString sInfo=QString(),QString sGUID=QString());
    bool removeBreakPoint(XADDR nAddress,BPT bpType=BPT_CODE_SOFTWARE);
    bool isBreakPointPresent(XADDR nAddress,BPT bpType=BPT_CODE_SOFTWARE);
    BREAKPOINT findBreakPointByAddress(XADDR nAddress,BPT bpType=BPT_CODE_SOFTWARE);
    BREAKPOINT findBreakPointByExceptionAddress(XADDR nExceptionAddress,BPT bpType=BPT_CODE_SOFTWARE);

    QList<BREAKPOINT> *getBreakpoints();
    QMap<X_ID,BREAKPOINT> *getThreadBreakpoints();
    bool breakpointToggle(XADDR nAddress);

    void addSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void removeSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);

    void addThreadInfo(XInfoDB::THREAD_INFO *pThreadInfo);
    void removeThreadInfo(X_ID nThreadID);

    bool setFunctionHook(QString sFunctionName);
    bool removeFunctionHook(QString sFunctionName);

    QMap<XADDR,SHAREDOBJECT_INFO> *getSharedObjectInfos();
    QList<THREAD_INFO> *getThreadInfos();
    QMap<QString,FUNCTIONHOOK_INFO> *getFunctionHookInfos();

    SHAREDOBJECT_INFO findSharedInfoByName(QString sName);
    SHAREDOBJECT_INFO findSharedInfoByAddress(XADDR nAddress);

    THREAD_INFO findThreadInfoByID(X_ID nThreadID);
#ifdef Q_OS_WIN
    THREAD_INFO findThreadInfoByHandle(X_HANDLE hThread);
#endif
    quint64 getFunctionAddress(QString sFunctionName);
    bool setSingleStep(X_HANDLE hThread,QString sInfo="");
//    bool stepInto(XProcess::HANDLEID handleThread);
//    bool resumeThread(XProcess::HANDLEID handleThread);
    bool stepIntoByHandle(X_HANDLE hThread);
    bool stepIntoById(X_ID nThreadId);
    bool _setStepByHandle(X_HANDLE hThread);
    bool _setStepById(X_ID nThreadId);
    bool suspendThread(X_HANDLE hThread);
    bool resumeThread(X_HANDLE hThread);
    bool suspendOtherThreads(X_ID nThreadId);
    bool resumeOtherThreads(X_ID nThreadId);
    bool suspendAllThreads();
    bool resumeAllThreads();
    FUNCTION_INFO getFunctionInfo(X_HANDLE hThread,QString sName);

    void _lockId(quint32 nId);
    void _unlockID(quint32 nId);
    void _waitID(quint32 nId);
    XBinary::XVARIANT getCurrentRegCache(XREG reg);
    bool setCurrentReg(X_HANDLE hThread,XREG reg,XBinary::XVARIANT variant);
    bool isRegChanged(XREG reg);

    XADDR getCurrentStackPointerCache();
    XADDR getCurrentInstructionPointerCache();

    XADDR getCurrentInstructionPointerByHandle(X_HANDLE hThread);
    XADDR getCurrentInstructionPointerById(X_ID nThreadId);
    bool setCurrentIntructionPointerByHandle(X_HANDLE hThread,XADDR nValue);

    XADDR getCurrentStackPointerByHandle(X_HANDLE hThread);
    XADDR getCurrentStackPointerById(X_ID nThreadId);
    bool setCurrentStackPointerByHandle(X_HANDLE hThread,XADDR nValue);

    static QString regIdToString(XREG reg);

    static XREG getSubReg32(XREG reg);
    static XREG getSubReg16(XREG reg);
    static XREG getSubReg8H(XREG reg);
    static XREG getSubReg8L(XREG reg);
#endif
    struct XSTRING
    {
        QString sAnsiString;
        QString sUnicodeString;
        QString sUTFString;
    };

    struct RECORD_INFO
    {
        bool bValid;
        XADDR nAddress;
        QString sModule;
        QByteArray baData;
        QString sSymbol;
        QString sInfo;
    };

    enum RI_TYPE
    {
        RI_TYPE_UNKNOWN=0,
        RI_TYPE_GENERAL,
        RI_TYPE_ADDRESS,
        RI_TYPE_DATA,
        RI_TYPE_ANSI,
        RI_TYPE_UNICODE,
        RI_TYPE_UTF8,
        RI_TYPE_SYMBOL,
        RI_TYPE_SYMBOLADDRESS
    };

    RECORD_INFO getRecordInfo(quint64 nValue,RI_TYPE riType=RI_TYPE_GENERAL);
    static QString recordInfoToString(RECORD_INFO recordInfo,RI_TYPE riType=RI_TYPE_GENERAL);

    void clearRecordInfoCache();
    RECORD_INFO getRecordInfoCache(quint64 nValue);

    QList<XBinary::MEMORY_REPLACE> getMemoryReplaces(quint64 nBase=0,quint64 nSize=0xFFFFFFFFFFFFFFFF);

    enum SS
    {
        SS_UNKNOWN=0,
        SS_FILE,
        SS_USER
    };

    enum ST
    {
        ST_UNKNOWN=0,
        ST_LABEL,
        ST_ENTRYPOINT,
        ST_EXPORT,
        ST_IMPORT,
        ST_DATA,
        ST_OBJECT,
        ST_FUNCTION
    };

    struct SYMBOL
    {
        XADDR nAddress;
        qint64 nSize;
        quint32 nModule; // ModuleIndex; 0 = main module
        QString sSymbol;
        ST symbolType;
        SS symbolSource;
    };

    QList<SYMBOL> *getSymbols();
    QMap<quint32,QString> *getSymbolModules();

    void addSymbol(XADDR nAddress,qint64 nSize,quint32 nModule,QString sSymbol,ST symbolTyp,SS symbolSource);
    void _addSymbol(XADDR nAddress,qint64 nSize,quint32 nModule,QString sSymbol,ST symbolType,SS symbolSource);
    void _sortSymbols();
    qint32 _getSymbolIndex(XADDR nAddress,qint64 nSize,quint32 nModule, qint32 *pnInsertIndex);

    static QString symbolSourceIdToString(SS symbolSource);
    static QString symbolTypeIdToString(ST symbolType);

signals:
    void dataChanged(bool bDataReload);

private:
#ifdef USE_XPROCESS
    struct STATUS
    {
        QMap<XREG,XBinary::XVARIANT> mapRegs;
    #ifdef USE_XPROCESS
        QList<XProcess::MEMORY_REGION> listMemoryRegions;
        QList<XProcess::MODULE> listModules;
    #endif
    };
    XBinary::XVARIANT _getRegCache(QMap<XREG,XBinary::XVARIANT> *pMapRegs,XREG reg);
#endif
private:
#ifdef USE_XPROCESS
    XInfoDB::PROCESS_INFO g_processInfo;
    QList<BREAKPOINT> g_listBreakpoints;
    QMap<X_ID,BREAKPOINT> g_mapThreadBreakpoints;         // STEPS, ThreadID/BP TODO QList
//    QMap<X_ID,BREAKPOINT> g_mapThreadBreakpoints;         // STEPS, ThreadID/BP TODO QList
    QMap<XADDR,SHAREDOBJECT_INFO> g_mapSharedObjectInfos;  // TODO QList
    QList<THREAD_INFO> g_listThreadInfos;
    QMap<QString,FUNCTIONHOOK_INFO> g_mapFunctionHookInfos; // TODO QList
#endif
    MODE g_mode;
#ifdef USE_XPROCESS
    STATUS g_statusCurrent;
    STATUS g_statusPrev;
#endif
    QList<SYMBOL> g_listSymbols;
    QMap<quint32,QString> g_mapSymbolModules;
    QMap<quint64,RECORD_INFO> g_mapSRecordInfoCache;
    QIODevice *g_pDevice;
    XBinary::FT g_fileType;
    XBinary::_MEMORY_MAP g_MainModuleMemoryMap;
    XADDR g_nMainModuleAddress;
    quint64 g_nMainModuleSize;
    QString g_sMainModuleName;
    QMap<quint32,QMutex *> g_mapIds;
};

#endif // XINFODB_H
