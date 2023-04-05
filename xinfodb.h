/* Copyright (c) 2022-2023 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <QMutex>

#include "xcapstone.h"
#include "xformats.h"
#ifdef USE_XPROCESS
#include "xprocess.h"
#endif

#ifdef QT_SQL_LIB
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#endif

class XInfoDB : public QObject {
    Q_OBJECT
public:
    enum MODE {
        MODE_UNKNOWN = 0,
        MODE_DEVICE,
#ifdef USE_XPROCESS
        MODE_PROCESS
#endif
    };
#ifdef USE_XPROCESS
    struct XREG_OPTIONS {
        bool bGeneral;
        bool bIP;
#ifdef Q_PROCESSOR_X86
        bool bFlags;
        bool bSegments;
        bool bDebug;
        bool bFloat;
        bool bXMM;
#endif
    };

    enum XREG {
        XREG_UNKNOWN = 0,
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

    enum BPT {
        BPT_UNKNOWN = 0,
        BPT_CODE_SOFTWARE,  // for X86 0xCC Check for ARM Check invalid opcodes
                            // as BP
        BPT_CODE_HARDWARE,
        BPT_CODE_MEMORY
        // TODO software invalid opcode
    };

    enum BPI {
        BPI_UNKNOWN = 0,
        BPI_SYSTEM,
        BPI_USER,
        BPI_PROCESSENTRYPOINT,
        BPI_PROGRAMENTRYPOINT,
        BPI_TLSFUNCTION,  // TODO
        BPI_FUNCTIONENTER,
        BPI_FUNCTIONLEAVE,
        BPI_STEPINTO,
        BPI_STEPOVER,
        BPI_TRACEINTO,
        BPI_TRACEOVER,
        BPI_STEPINTO_RESTOREBP
    };

    struct BREAKPOINT {
        // TODO bIsValid
        XADDR nAddress;
        qint64 nSize;
        qint32 nCount;
        BPT bpType;
        BPI bpInfo;
        QString sInfo;
        qint32 nOrigDataSize;
        char origData[4];  // TODO consts check
        QString sGUID;
    };

    enum THREAD_STATUS {
        THREAD_STATUS_UNKNOWN = 0,
        THREAD_STATUS_PAUSED,
        THREAD_STATUS_RUNNING
    };

    struct THREAD_INFO {
        X_ID nThreadID;
        qint64 nThreadLocalBase;
        XADDR nStartAddress;
#ifdef Q_OS_WIN
        X_HANDLE hThread;
#endif
        THREAD_STATUS threadStatus;
    };

    struct EXITTHREAD_INFO {
        qint64 nThreadID;
        qint64 nExitCode;
    };

    struct PROCESS_INFO {
        qint64 nProcessID;
        qint64 nMainThreadID;  // TODO Check mb Remove
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

    struct EXITPROCESS_INFO {
        qint64 nProcessID;
        qint64 nThreadID;
        qint64 nExitCode;
    };

    struct SHAREDOBJECT_INFO  // DLL on Windows
    {
        QString sName;
        QString sFileName;
        XADDR nImageBase;
        quint64 nImageSize;
    };

    struct DEBUGSTRING_INFO {
        qint64 nThreadID;
        QString sDebugString;
    };

    struct BREAKPOINT_INFO {
        XADDR nAddress;
        XInfoDB::BPT bpType;
        XInfoDB::BPI bpInfo;
        QString sInfo;
        X_ID nProcessID;
#ifdef Q_OS_WIN
        X_HANDLE hProcess;
#endif
#ifdef Q_OS_LINUX
        X_HANDLE_IO pHProcessMemoryIO;     // TODO rename
        X_HANDLE_MQ pHProcessMemoryQuery;  // TODO rename
#endif
#ifdef Q_OS_MACOS
        X_HANDLE hProcess;
#endif
        X_ID nThreadID;
#ifdef Q_OS_WIN
        X_HANDLE hThread;
#endif
    };

    struct PROCESSENTRY_INFO {
        XADDR nAddress;
    };

    struct FUNCTIONHOOK_INFO {
        QString sName;
        XADDR nAddress;
    };

    struct FUNCTION_INFO {
        QString sName;
        XADDR nAddress;
        XADDR nRetAddress;
        quint64 nParameters[10];  // TODO const mb TODO number of parametrs
    };
#endif

    explicit XInfoDB(QObject *pParent = nullptr);
    ~XInfoDB();

    void setDevice(QIODevice *pDevice, XBinary::FT fileType = XBinary::FT_UNKNOWN);
    QIODevice *getDevice();
    void setFileType(XBinary::FT fileType);
    XBinary::FT getFileType();
    void setDisasmMode(XBinary::DM disasmMode);
    XBinary::DM getDisasmMode();
    void reload(bool bReloadData);
    void setEdited(qint64 nDeviceOffset, qint64 nDeviceSize);
    void _createTableNames();
#ifdef USE_XPROCESS
    quint32 read_uint32(XADDR nAddress, bool bIsBigEndian = false);
    quint64 read_uint64(XADDR nAddress, bool bIsBigEndian = false);
    qint64 read_array(XADDR nAddress, char *pData, quint64 nSize);
    qint64 write_array(XADDR nAddress, char *pData, quint64 nSize);
    QByteArray read_array(XADDR nAddress, quint64 nSize);
    QString read_ansiString(XADDR nAddress, quint64 nMaxSize = 256);
    QString read_unicodeString(XADDR nAddress,
                               quint64 nMaxSize = 256);  // TODO endian ??
    QString read_utf8String(XADDR nAddress, quint64 nMaxSize = 256);
    XCapstone::DISASM_RESULT disasm(XADDR nAddress);
#else
    quint32 read_uint32(qint64 nOffset, bool bIsBigEndian = false);
    quint64 read_uint64(qint64 nOffset, bool bIsBigEndian = false);
    qint64 read_array(qint64 nOffset, char *pData, quint64 nSize);
    qint64 write_array(qint64 nOffset, char *pData, quint64 nSize);
    QByteArray read_array(qint64 nOffset, quint64 nSize);
    QString read_ansiString(qint64 nOffset, quint64 nMaxSize = 256);
    QString read_unicodeString(qint64 nOffset, quint64 nMaxSize = 256);  // TODO endian ??
    QString read_utf8String(qint64 nOffset, quint64 nMaxSize = 256);
#endif

    enum STRDB {
        STRDB_UNKNOWN = 0,
        STRDB_PESECTIONS
    };

    static QList<QString> getStringsFromFile(QString sFileName);

    struct STRRECORD {
        QString sString;
        QString sType;
        QString sDescription;
    };

    static STRRECORD handleStringDB(QList<QString> *pListStrings, QString sString, bool bIsMulti);
    static QList<QString> loadStrDB(QString sPath, STRDB strDB);
#ifdef USE_XPROCESS
    void setProcessInfo(PROCESS_INFO processInfo);
    PROCESS_INFO *getProcessInfo();
    void setCurrentThreadId(X_ID nThreadId);
    void setCurrentThreadHandle(X_HANDLE hThread);
    X_ID getCurrentThreadId();
    X_HANDLE getCurrentThreadHandle();
    void updateRegsById(X_ID nThreadId, XREG_OPTIONS regOptions);
    void updateRegsByHandle(X_HANDLE hThread, XREG_OPTIONS regOptions);
    void updateMemoryRegionsList();
    void updateModulesList();
    void updateThreadsList();
    QList<XProcess::MEMORY_REGION> *getCurrentMemoryRegionsList();
    QList<XProcess::MODULE> *getCurrentModulesList();
    QList<XProcess::THREAD_INFO> *getCurrentThreadsList();
    bool addBreakPoint(XADDR nAddress, BPT bpType = BPT_CODE_SOFTWARE, BPI bpInfo = BPI_UNKNOWN, qint32 nCount = -1, QString sInfo = QString(),
                       QString sGUID = QString());
    bool removeBreakPoint(XADDR nAddress, BPT bpType = BPT_CODE_SOFTWARE);
    bool isBreakPointPresent(XADDR nAddress, BPT bpType = BPT_CODE_SOFTWARE);
    BREAKPOINT findBreakPointByAddress(XADDR nAddress, BPT bpType = BPT_CODE_SOFTWARE);
    BREAKPOINT findBreakPointByExceptionAddress(XADDR nExceptionAddress, BPT bpType = BPT_CODE_SOFTWARE);

    QList<BREAKPOINT> *getBreakpoints();
#ifdef Q_OS_WIN
    QMap<X_HANDLE, BREAKPOINT> *getThreadBreakpoints();
#endif
#ifdef Q_OS_LINUX
    QMap<X_ID, BREAKPOINT> *getThreadBreakpoints();
#endif
    bool breakpointToggle(XADDR nAddress);

    void addSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void removeSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);

    void addThreadInfo(XInfoDB::THREAD_INFO *pThreadInfo);
    void removeThreadInfo(X_ID nThreadID);

    bool setFunctionHook(QString sFunctionName);
    bool removeFunctionHook(QString sFunctionName);

    QMap<XADDR, SHAREDOBJECT_INFO> *getSharedObjectInfos();
    QList<THREAD_INFO> *getThreadInfos();
    QMap<QString, FUNCTIONHOOK_INFO> *getFunctionHookInfos();

    SHAREDOBJECT_INFO findSharedInfoByName(QString sName);
    SHAREDOBJECT_INFO findSharedInfoByAddress(XADDR nAddress);

    THREAD_INFO findThreadInfoByID(X_ID nThreadID);
#ifdef Q_OS_WIN
    THREAD_INFO findThreadInfoByHandle(X_HANDLE hThread);
#endif
    quint64 getFunctionAddress(QString sFunctionName);
    bool setSingleStep(X_HANDLE hThread, QString sInfo = "");  // TODO mb remove
    //    bool stepInto(XProcess::HANDLEID handleThread);
    //    bool resumeThread(XProcess::HANDLEID handleThread);
    XADDR getAddressNextInstructionAfterCall(XADDR nAddress);
    bool stepInto_Handle(X_HANDLE hThread, BPI bpInfo, bool bAddThreadBP);
    bool stepInto_Id(X_ID nThreadId, BPI bpInfo, bool bAddThreadBP);
    bool stepOver_Handle(X_HANDLE hThread, BPI bpInfo, bool bAddThreadBP);
    bool stepOver_Id(X_ID nThreadId, BPI bpInfo, bool bAddThreadBP);
    bool _setStep_Handle(X_HANDLE hThread);
    bool _setStep_Id(X_ID nThreadId);
    bool suspendThread_Id(X_ID nThreadId);
    bool suspendThread_Handle(X_HANDLE hThread);
    bool resumeThread_Handle(X_HANDLE hThread);
    bool suspendOtherThreads(X_ID nThreadId);
    bool resumeOtherThreads(X_ID nThreadId);
    bool suspendAllThreads();
    bool resumeAllThreads();
    FUNCTION_INFO getFunctionInfo(X_HANDLE hThread, QString sName);

    //    void _lockId(quint32 nId);
    //    void _unlockID(quint32 nId);
    //    void _waitID(quint32 nId);
    XBinary::XVARIANT getCurrentRegCache(XREG reg);
    void setCurrentRegCache(XREG reg, XBinary::XVARIANT variant);
    bool setCurrentRegByThread(X_HANDLE hThread, XREG reg, XBinary::XVARIANT variant);
    bool setCurrentRegById(X_ID nThreadId, XREG reg, XBinary::XVARIANT variant);
    bool setCurrentReg(XREG reg, XBinary::XVARIANT variant);
    bool isRegChanged(XREG reg);

    XADDR getCurrentStackPointerCache();
    XADDR getCurrentInstructionPointerCache();

    XADDR getCurrentInstructionPointer_Handle(X_HANDLE hThread);
    XADDR getCurrentInstructionPointer_Id(X_ID nThreadId);
    bool setCurrentIntructionPointer_Handle(X_HANDLE hThread, XADDR nValue);
    bool setCurrentIntructionPointer_Id(X_ID nThreadId, XADDR nValue);

    XCapstone::OPCODE_ID getCurrentOpcode_Handle(X_HANDLE hThread);
    XCapstone::OPCODE_ID getCurrentOpcode_Id(X_ID nThreadId);

    XADDR getCurrentStackPointer_Handle(X_HANDLE hThread);
    XADDR getCurrentStackPointer_Id(X_ID nThreadId);
    bool setCurrentStackPointer_Handle(X_HANDLE hThread, XADDR nValue);

    static QString regIdToString(XREG reg);

    static XREG getSubReg32(XREG reg);
    static XREG getSubReg16(XREG reg);
    static XREG getSubReg8H(XREG reg);
    static XREG getSubReg8L(XREG reg);
#endif
    struct XSTRING {
        QString sAnsiString;
        QString sUnicodeString;
        QString sUTFString;
    };

    struct RECORD_INFO {
        bool bValid;
        XADDR nAddress;
        QString sModule;
        QByteArray baData;
        QString sSymbol;
        QString sInfo;
    };

    enum RI_TYPE {
        RI_TYPE_UNKNOWN = 0,
        RI_TYPE_GENERAL,
        RI_TYPE_ADDRESS,
        RI_TYPE_DATA,
        RI_TYPE_ANSI,
        RI_TYPE_UNICODE,
        RI_TYPE_UTF8,
        RI_TYPE_SYMBOL,
        RI_TYPE_SYMBOLADDRESS
    };

    RECORD_INFO getRecordInfo(quint64 nValue, RI_TYPE riType = RI_TYPE_GENERAL);
    static QString recordInfoToString(RECORD_INFO recordInfo, RI_TYPE riType = RI_TYPE_GENERAL);
    void clearRecordInfoCache();
    RECORD_INFO getRecordInfoCache(quint64 nValue);
    QList<XBinary::MEMORY_REPLACE> getMemoryReplaces(quint64 nBase = 0, quint64 nSize = 0xFFFFFFFFFFFFFFFF);

    enum RT {
        RT_UNKNOWN = 0,
        RT_CODE,
        RT_DATA
    };

    struct SYMBOL {
        XADDR nAddress;
        quint32 nModule;  // ModuleIndex; 0 - main module
        QString sSymbol;
    };

    struct SHOWRECORD {
        XADDR nAddress;
        qint64 nOffset;
        qint64 nSize;
        QString sRecText1;
        QString sRecText2;
        RT recordType;
        quint64 nLineNumber;
        quint64 nRefTo;
        quint64 nRefFrom;
    };

    struct RELRECORD {
        XADDR nAddress;
        XCapstone::RELTYPE relType;
        XADDR nXrefToRelative;
        XCapstone::MEMTYPE memType;
        XADDR nXrefToMemory;
        qint32 nMemorySize;
    };

    bool isSymbolsPresent();
    QList<SYMBOL> getAllSymbols();
    QMap<quint32, QString> getSymbolModules();

    //    QList<XADDR> getSymbolAddresses(ST symbolType);

    void addSymbol(XADDR nAddress, quint32 nModule, QString sSymbol);
    bool _addSymbol(XADDR nAddress, quint32 nModule, QString sSymbol);
    void _sortSymbols();
    qint32 _getSymbolIndex(XADDR nAddress, qint64 nSize, quint32 nModule, qint32 *pnInsertIndex);

    bool _addExportSymbol(XADDR nAddress, QString sSymbol);
    bool _addImportSymbol(XADDR nAddress, QString sSymbol);
    bool _addTLSSymbol(XADDR nAddress, QString sSymbol);

    //    static QString symbolSourceIdToString(SS symbolSource);
    //    static QString symbolTypeIdToString(ST symbolType);

    SYMBOL getSymbolByAddress(XADDR nAddress);
    bool isSymbolPresent(XADDR nAddress);
    QString getSymbolStringByAddress(XADDR nAddress);
    void initDb();
#ifdef QT_SQL_LIB
    void initDb(QSqlDatabase *pDatabase);
#endif
    void clearDb();
    void vacuumDb();
    void _addSymbols(QIODevice *pDevice, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct = nullptr);
    void _disasmAnalyze(QIODevice *pDevice, XBinary::_MEMORY_MAP *pMemoryMap, XADDR nStartAddress, bool bIsInit, XBinary::PDSTRUCT *pPdStruct = nullptr);
    bool _addShowRecord(XADDR nAddress, qint64 nOffset, qint64 nSize, QString sRecText1, QString sRecText2, RT recordType, qint64 nLineNumber, quint64 nRefTo,
                        quint64 nRefFrom);
    bool _isShowRecordPresent(XADDR nAddress, qint64 nSize);
    bool _addRelRecord(XADDR nAddress, XCapstone::RELTYPE relType, XADDR nXrefToRelative, XCapstone::MEMTYPE memType, XADDR nXrefToMemory, qint32 nMemorySize);
    void _addShowRecords(QList<SHOWRECORD> *pListRecords);
    void _addRelRecords(QList<RELRECORD> *pListRecords);
    QList<RELRECORD> getRelRecords();
    bool _incShowRecordRefFrom(XADDR nAddress);
    bool _addBookmark(quint64 nLocation, qint64 nSize, QColor colText, QColor colBackground, QString sName, QString sComment);

    SHOWRECORD getShowRecordByAddress(XADDR nAddress);
    SHOWRECORD getNextShowRecordByAddress(XADDR nAddress);
    SHOWRECORD getPrevShowRecordByAddress(XADDR nAddress);
    SHOWRECORD getShowRecordByLine(qint64 nLine);
    SHOWRECORD getShowRecordByOffset(qint64 nOffset);
    qint64 getShowRecordOffsetByAddress(XADDR nAddress);
    qint64 getShowRecordPrevOffsetByAddress(XADDR nAddress);
    qint64 getShowRecordOffsetByLine(qint64 nLine);
    XADDR getShowRecordAddressByOffset(qint64 nOffset);
    XADDR getShowRecordAddressByLine(qint64 nLine);
    qint64 getShowRecordsCount();
    qint64 getShowRecordLineByAddress(XADDR nAddress);
    qint64 getShowRecordLineByOffset(qint64 nOffset);
    void updateShowRecordLine(XADDR nAddress, qint64 nLine);
    QList<SHOWRECORD> getShowRecords(qint64 nLine, qint32 nCount);

    QList<XADDR> getShowRecordRelAddresses(XCapstone::RELTYPE relType);
    QList<XBinary::ADDRESSSIZE> getShowRecordMemoryVariables();

    QList<XADDR> getExportSymbolAddresses();
    QList<XADDR> getImportSymbolAddresses();
    QList<XADDR> getTLSSymbolAddresses();

    RELRECORD getRelRecordByAddress(XADDR nAddress);
    bool isAddressHasRefFrom(XADDR nAddress);

    bool isAnalyzedRegionVirtual(XADDR nAddress, qint64 nSize);

    void setAnalyzed(bool bState);
    bool isAnalyzed();
    bool isAnalyzeInProgress();

    void disasmToDb(qint64 nOffset, XCapstone::DISASM_RESULT disasmResult);
    XCapstone::DISASM_RESULT dbToDisasm(XADDR nAddress);

    bool loadDbFromFile(QString sDBFileName, XBinary::PDSTRUCT *pPdStruct = nullptr);
    bool saveDbToFile(QString sDBFileName, XBinary::PDSTRUCT *pPdStruct = nullptr);

#ifdef QT_SQL_LIB
    bool querySQL(QSqlQuery *pSqlQuery, QString sSQL);
    bool querySQL(QSqlQuery *pSqlQuery);
    QString convertStringSQL(QString sSQL);
    bool copyDb(QSqlDatabase *pDatabaseSource, QSqlDatabase *pDatabaseDest, XBinary::PDSTRUCT *pPdStruct);
#endif
    void testFunction();

    void setDebuggerState(bool bState);
    bool isDebugger();

public slots:
    void readDataSlot(quint64 nOffset, char *pData, qint64 nSize);
    void writeDataSlot(quint64 nOffset, char *pData, qint64 nSize);

private:
    void replaceMemory(quint64 nOffset, char *pData, qint64 nSize);

signals:
    void dataChanged(qint64 nDeviceOffset, qint64 nDeviceSize);
    void reloadSignal(bool bReloadData);
    void memoryRegionsListChanged();
    void modulesListChanged();
    void threadsListChanged();
    void registersListChanged();
    //    void analyzeStateChanged();

private:
#ifdef USE_XPROCESS
    struct STATUS {
        quint32 nRegistersHash;
        QMap<XREG, XBinary::XVARIANT> mapRegs;
        QMap<XREG, XBinary::XVARIANT> mapRegsPrev;  // mb TODO move to prev
        X_ID nThreadId;
        X_HANDLE hThread;
        quint32 nMemoryRegionsHash;
        QList<XProcess::MEMORY_REGION> listMemoryRegions;
        quint32 nModulesHash;
        quint32 nThreadsHash;
        QList<XProcess::MODULE> listModules;
        QList<XProcess::THREAD_INFO> listThreads;
    };
    XBinary::XVARIANT _getRegCache(QMap<XREG, XBinary::XVARIANT> *pMapRegs, XREG reg);
    void _setRegCache(QMap<XREG, XBinary::XVARIANT> *pMapRegs, XREG reg, XBinary::XVARIANT variant);
#endif
private:
#ifdef USE_XPROCESS
    XInfoDB::PROCESS_INFO g_processInfo;

    QList<BREAKPOINT> g_listBreakpoints;
#ifdef Q_OS_WIN
    QMap<X_HANDLE, BREAKPOINT> g_mapThreadBreakpoints;  // STEPS, ThreadID/BP TODO QList
#endif
#ifdef Q_OS_LINUX
    QMap<X_ID, BREAKPOINT> g_mapThreadBreakpoints;  // STEPS, ThreadID/BP TODO QList
#endif
    //    QMap<X_ID,BREAKPOINT> g_mapThreadBreakpoints;         // STEPS,
    //    ThreadID/BP TODO QList
    QMap<XADDR, SHAREDOBJECT_INFO> g_mapSharedObjectInfos;  // TODO QList
    QList<THREAD_INFO> g_listThreadInfos;
    QMap<QString, FUNCTIONHOOK_INFO> g_mapFunctionHookInfos;  // TODO QList
#endif
    MODE g_mode;
#ifdef USE_XPROCESS
    STATUS g_statusCurrent;
//    STATUS g_statusPrev;
#endif
#ifndef QT_SQL_LIB
    QList<SYMBOL> g_listSymbols;  // TODO remove
#endif
    QMap<quint32, QString> g_mapSymbolModules;  // TODO move to SQL
    QMap<quint64, RECORD_INFO> g_mapSRecordInfoCache;
    QIODevice *g_pDevice;
    XBinary g_binary;
    XBinary::FT g_fileType;
    XBinary::DM g_disasmMode;
    csh g_handle;
    XBinary::_MEMORY_MAP g_MainModuleMemoryMap;
    XADDR g_nMainModuleAddress;
    quint64 g_nMainModuleSize;
    QString g_sMainModuleName;
    QMap<quint32, QMutex *> g_mapIds;
#ifdef QT_SQL_LIB
    QSqlDatabase g_dataBase;
    QString s_sql_symbolTableName;
    QString s_sql_recordTableName;
    QString s_sql_relativeTableName;
    QString s_sql_importTableName;
    QString s_sql_exportTableName;
    QString s_sql_tlsTableName;
    QString s_sql_bookmarksTableName;
#endif
    bool g_bIsAnalyzed;
    bool g_bIsAnalyzeInProgress;
    bool g_bIsDebugger;
};

#endif  // XINFODB_H
