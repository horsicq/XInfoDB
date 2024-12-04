/* Copyright (c) 2022-2024 hors<horsicq@gmail.com>
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
#ifdef QT_GUI_LIB
#include <QColor>
#endif
#include "xcapstone.h"
#include "xformats.h"
#include "xscanengine.h"
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
        bool bYMM;
#endif
    };

    struct XHARDWAREBPREG {
        XADDR nAddress;
        bool bLocal;
        bool bGlobal;
        qint32 nSize;
        bool bRead;
        bool bWrite;
        bool bExec;
    };

    struct XHARDWAREBP {
        XHARDWAREBPREG regs[4];
        bool bSuccess[4];
        bool bSingleStep;
        // TODO Check more
    };

#ifdef Q_PROCESSOR_X86
    enum RFLAGS_BIT {
        RFLAGS_BIT_CF = 0,
        RFLAGS_BIT_PF = 2,
        RFLAGS_BIT_AF = 4,
        RFLAGS_BIT_ZF = 6,
        RFLAGS_BIT_SF = 7,
        RFLAGS_BIT_TF = 8,
        RFLAGS_BIT_IF = 9,
        RFLAGS_BIT_DF = 10,
        RFLAGS_BIT_OF = 11
    };
#endif

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
        XREG_FLAGS_CF,
        XREG_FLAGS_PF,
        XREG_FLAGS_AF,
        XREG_FLAGS_ZF,
        XREG_FLAGS_SF,
        XREG_FLAGS_TF,
        XREG_FLAGS_IF,
        XREG_FLAGS_DF,
        XREG_FLAGS_OF,
        XREG_ST0,
        XREG_ST1,
        XREG_ST2,
        XREG_ST3,
        XREG_ST4,
        XREG_ST5,
        XREG_ST6,
        XREG_ST7,
        XREG_FPCR,  // https://help.totalview.io/previous_releases/2019/html/index.html#page/Reference_Guide%2FIntelx86FloatingPointRegisters_2.html%23
        XREG_FPSR,
        XREG_FPTAG,
        //        XREG_FPIOFF,
        //        XREG_FPISEL,
        //        XREG_FPDOFF,
        //        XREG_FPDSEL,
        XREG_MXCSR,
        XREG_MXCSR_MASK,
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
        XREG_YMM0,
        XREG_YMM1,
        XREG_YMM2,
        XREG_YMM3,
        XREG_YMM4,
        XREG_YMM5,
        XREG_YMM6,
        XREG_YMM7,
        XREG_YMM8,
        XREG_YMM9,
        XREG_YMM10,
        XREG_YMM11,
        XREG_YMM12,
        XREG_YMM13,
        XREG_YMM14,
        XREG_YMM15,
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

    struct REG_RECORD {
        XREG reg;
        XBinary::XVARIANT value;
    };

    enum BPT {
        BPT_UNKNOWN = 0,
        BPT_PROCESS_STOP,
        BPT_CODE_SYSTEM_EXCEPTION,
        BPT_CODE_STEP_FLAG,
        BPT_CODE_STEP_TO_RESTORE,
        BPT_CODE_SOFTWARE_DEFAULT,
        BPT_CODE_SOFTWARE_INT1,
        BPT_CODE_SOFTWARE_INT3,  // for X86 0xCC Check for ARM Check invalid opcodes
        BPT_CODE_SOFTWARE_HLT,
        BPT_CODE_SOFTWARE_CLI,
        BPT_CODE_SOFTWARE_STI,
        BPT_CODE_SOFTWARE_INSB,
        BPT_CODE_SOFTWARE_INSD,
        BPT_CODE_SOFTWARE_OUTSB,
        BPT_CODE_SOFTWARE_OUTSD,
        BPT_CODE_SOFTWARE_INT1LONG,
        BPT_CODE_SOFTWARE_INT3LONG,
        BPT_CODE_SOFTWARE_UD0,
        BPT_CODE_SOFTWARE_UD2,
        BPT_CODE_HARDWARE_FREE,  // Check free Debug register
        BPT_CODE_HARDWARE_DR0,
        BPT_CODE_HARDWARE_DR1,
        BPT_CODE_HARDWARE_DR2,
        BPT_CODE_HARDWARE_DR3,
        BPT_CODE_MEMORY
    };

    enum BPM {
        BPM_EXECUTE = 0,
        BPM_MEMORY
    };

    enum BPI {
        BPI_UNKNOWN = 0,
        BPI_SYSTEM,
        BPI_USER,
        BPI_TOGGLE,
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
        bool bIsEnable;  // TODO
        XADDR nAddress;
        qint64 nSize;
        X_ID nThreadID;
        bool bOneShot;
        BPT bpType;
        BPM bpMode;
        BPI bpInfo;
        QVariant vInfo;  // TODO rename
        qint32 nDataSize;
        char origData[4];  // TODO consts check
        char bpData[4];
        QString sUUID;
    };

    enum THREAD_STATUS {
        THREAD_STATUS_UNKNOWN = 0,
        THREAD_STATUS_CURRENT,
        THREAD_STATUS_PAUSED,
        THREAD_STATUS_RUNNING  // TODO
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
        qint32 hProcessMemoryIO;
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
        QString sFileName;
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
        XADDR nExceptionAddress;
        XInfoDB::BPT bpType;
        XInfoDB::BPI bpInfo;
        QVariant vInfo;
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

#ifdef QT_SQL_LIB
    enum DBTABLE {
        DBTABLE_BOOKMARKS = 0,
        DBTABLE_SHOWRECORDS,
        DBTABLE_RELATIVS,
        DBTABLE_IMPORT,
        DBTABLE_EXPORT,
        DBTABLE_TLS,
        DBTABLE_SYMBOLS,
        DBTABLE_FUNCTIONS,
        __DBTABLE_SIZE
    };
#endif

    struct IAT_RECORD {
        XADDR nAddress;
        XADDR nValue;
        QString sFunction;  // Library#Function / Library#Ordinal
    };

    explicit XInfoDB(QObject *pParent = nullptr);
    ~XInfoDB();

    void setData(QIODevice *pDevice, XBinary::FT fileType = XBinary::FT_UNKNOWN, XBinary::DM disasmMode = XBinary::DM_UNKNOWN);
    QIODevice *getDevice();
    void initDB();
    XBinary::FT getFileType();
    XBinary::DM getDisasmMode();
    void reload(bool bReloadData);
    void reloadView();
    void setEdited(qint64 nDeviceOffset, qint64 nDeviceSize);
    void _createTableNames();
#ifdef USE_XPROCESS
    quint32 read_uint32(XADDR nAddress, bool bIsBigEndian = false);
    quint64 read_uint64(XADDR nAddress, bool bIsBigEndian = false);
    qint64 read_array(XADDR nAddress, char *pData, quint64 nSize);
    qint64 write_array(XADDR nAddress, char *pData, quint64 nSize);
    QByteArray read_array(XADDR nAddress, quint64 nSize);
    QString read_ansiString(XADDR nAddress, quint64 nMaxSize = 256);
    QString read_unicodeString(XADDR nAddress, quint64 nMaxSize = 256);  // TODO endian ??
    QString read_utf8String(XADDR nAddress, quint64 nMaxSize = 256);
    XCapstone::DISASM_RESULT disasm(XADDR nAddress);
    qint64 read_userData(X_ID nThreadId, qint64 nOffset, char *pData, qint64 nSize);
    qint64 write_userData(X_ID nThreadId, qint64 nOffset, char *pData, qint64 nSize);
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
        STRDB_LIBRARIES,
        STRDB_FUNCTIONS,
        STRDB_PESECTIONS,
        STRDB_ELFSECTIONS
    };

    static QList<QString> getStringsFromFile(const QString &sFileName, XBinary::PDSTRUCT *pPdStruct = nullptr);

    struct STRRECORD {
        QString sString;
        QString sTags;
        QString sType;
        QString sDescription;
    };

    static STRRECORD handleStringDB(QList<QString> *pListStrings, STRDB strDB, const QString &sString, bool bIsMulti, XBinary::PDSTRUCT *pPdStruct = nullptr);
    static QList<QString> loadStrDB(const QString &sPath, STRDB strDB, XBinary::PDSTRUCT *pPdStruct = nullptr);
#ifdef USE_XPROCESS
    void setDefaultBreakpointType(BPT bpType);
    void setProcessInfo(PROCESS_INFO processInfo);
    PROCESS_INFO *getProcessInfo();
    void setCurrentThreadId(X_ID nThreadId);
    void setCurrentThreadHandle(X_HANDLE hThread);
    X_ID getCurrentThreadId();
    X_HANDLE getCurrentThreadHandle();
    void updateRegsById(X_ID nThreadId, const XREG_OPTIONS &regOptions);
    void updateRegsByHandle(X_HANDLE hThread, const XREG_OPTIONS &regOptions);
    void updateMemoryRegionsList();
    void updateModulesList();
    void updateThreadsList();
    QList<XProcess::MEMORY_REGION> *getCurrentMemoryRegionsList();
    QList<XProcess::MODULE> *getCurrentModulesList();
    QList<XProcess::THREAD_INFO> *getCurrentThreadsList();
    bool addBreakPoint(const BREAKPOINT &breakPoint);
    bool removeBreakPoint(const QString &sUUID);
    bool isBreakPointPresent(const BREAKPOINT &breakPoint);
    bool enableBreakPoint(const QString &sUUID);
    bool disableBreakPoint(const QString &sUUID);
    BREAKPOINT findBreakPointByAddress(XADDR nAddress, BPT bpType);
    BREAKPOINT findBreakPointByExceptionAddress(XADDR nExceptionAddress, BPT bpType);  // TODO try in *nix
    BREAKPOINT findBreakPointByThreadID(X_ID nThreadID, BPT bpType);
    BREAKPOINT findBreakPointByUUID(const QString &sUUID);
    BREAKPOINT findBreakPointByRegion(XADDR nAddress, qint64 nSize);
    qint32 getThreadBreakpointsCount(X_ID nThreadID);
    QList<BREAKPOINT> *getBreakpoints();
#ifdef Q_OS_LINUX
    QMap<X_ID, BREAKPOINT> *getThreadBreakpoints();
#endif
    bool breakpointToggle(XADDR nAddress);
    bool breakpointRemove(XADDR nAddress);
    static QString bptToString(BPT bpType);
    static QString bpiToString(BPI bpInfo);

    void addSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void removeSharedObjectInfo(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void addThreadInfo(XInfoDB::THREAD_INFO *pThreadInfo);
    void removeThreadInfo(X_ID nThreadID);
    bool setThreadStatus(X_ID nThreadID, THREAD_STATUS status);
    THREAD_STATUS getThreadStatus(X_ID nThreadID);
    bool setFunctionHook(const QString &sFunctionName);
    bool removeFunctionHook(const QString &sFunctionName);

    QMap<XADDR, SHAREDOBJECT_INFO> *getSharedObjectInfos();
    QList<THREAD_INFO> *getThreadInfos();
    QMap<QString, FUNCTIONHOOK_INFO> *getFunctionHookInfos();
    SHAREDOBJECT_INFO findSharedInfoByName(const QString &sName);
    SHAREDOBJECT_INFO findSharedInfoByAddress(XADDR nAddress);
    THREAD_INFO findThreadInfoByID(X_ID nThreadID);
#ifdef Q_OS_WIN
    THREAD_INFO findThreadInfoByHandle(X_HANDLE hThread);
#endif
    quint64 getFunctionAddress(const QString &sFunctionName);
    //    bool stepInto(XProcess::HANDLEID handleThread);
    //    bool resumeThread(XProcess::HANDLEID handleThread);
    XADDR getAddressNextInstructionAfterCall(XADDR nAddress);
    bool stepInto_Handle(X_HANDLE hThread, BPI bpInfo);
    bool stepInto_Id(X_ID nThreadId, BPI bpInfo);
    bool stepOver_Handle(X_HANDLE hThread, BPI bpInfo);
    bool stepOver_Id(X_ID nThreadId, BPI bpInfo);
    bool _setStep_Handle(X_HANDLE hThread);
    bool _setStep_Id(X_ID nThreadId);
    bool suspendThread_Id(X_ID nThreadId);
    bool suspendThread_Handle(X_HANDLE hThread);
    bool resumeThread_Id(X_ID nThreadId);
    bool resumeThread_Handle(X_HANDLE hThread);
    bool suspendAllThreads();
    bool resumeAllThreads();
    FUNCTION_INFO getFunctionInfo(X_HANDLE hThread, const QString &sName);

    XHARDWAREBP getHardwareBP_Handle(X_HANDLE hThread);
    //    bool setHardwareBP_Handle(X_HANDLE hThread, XHARDWAREBP &hardwareBP);
    XHARDWAREBP getHardwareBP_Id(X_ID nThreadId);
    bool setHardwareBP_Id(X_ID nThreadId, XHARDWAREBP &hardwareBP);
    bool _regsToXHARDWAREBP(quint64 *pDebugRegs, XHARDWAREBP *pHardwareBP);
    XHARDWAREBPREG _bitsToXHARDWAREBP(quint64 nReg, bool bLocal, bool bGlobal, bool bCond0, bool bCond1, bool bSize0, bool bSize1);
    //    bool _XHARDWAREBPToRegs(XHARDWAREBP *pHardwareBP, quint64 *pDebugRegs); // TODO Check

    //    void _lockId(quint32 nId);
    //    void _unlockID(quint32 nId);
    //    void _waitID(quint32 nId);
    XBinary::XVARIANT getCurrentRegCache(XREG reg);
    void setCurrentRegCache(XREG reg, XBinary::XVARIANT variant);
    bool setCurrentRegByThread(X_HANDLE hThread, XREG reg, XBinary::XVARIANT variant);
    bool setCurrentRegById(X_ID nThreadId, XREG reg, XBinary::XVARIANT variant);
    bool setCurrentReg(XREG reg, XBinary::XVARIANT variant);
    bool isRegChanged(XREG reg);
    QList<REG_RECORD> getCurrentRegs();
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
    bool isFunctionReturnAddress(XADDR nAddress);

    bool isAddressValid(XADDR nAddress);

    static QString regIdToString(XREG reg);
    static XREG getSubReg32(XREG reg);
    static XREG getSubReg16(XREG reg);
    static XREG getSubReg8H(XREG reg);
    static XREG getSubReg8L(XREG reg);

    static XREG_OPTIONS getRegOptions(XREG reg);

    static char *allocateStringMemory(const QString &sFileName);
    static XBinary::XVARIANT getFlagFromReg(XBinary::XVARIANT variant, XREG reg);
    static XBinary::XVARIANT setFlagToReg(XBinary::XVARIANT variant, XREG reg, bool bValue);
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
        RT_INTDATATYPE,
        RT_VIRTUAL
    };

    enum DBSTATUS {
        DBSTATUS_NONE,
        DBSTATUS_PROCESS
    };

    enum SS {
        SS_FILE,
        SS_ANALYZE
    };

    struct SYMBOL {
        XADDR nAddress;
        quint32 nModule;  // ModuleIndex; 0 - main module
        QString sSymbol;
        SS symSource;
    };

    struct REFERENCE {
        XADDR nAddress;
        QString sCode;
    };

    struct FUNCTION {
        XADDR nAddress;
        qint64 nSize;
        QString sName;
    };

    struct SHOWRECORD {
        bool bValid;
        XADDR nAddress;
        qint64 nOffset;
        qint64 nSize;
        RT recordType;
        quint64 nRefTo;
        quint64 nRefFrom;
        quint64 nBranch;
        DBSTATUS dbstatus;
    };

    struct RELRECORD {
        XADDR nAddress;
        XCapstone::RELTYPE relType;
        XADDR nXrefToRelative;
        XCapstone::MEMTYPE memType;
        XADDR nXrefToMemory;
        qint32 nMemorySize;
        DBSTATUS dbstatus;
    };
#ifdef QT_GUI_LIB
    struct BOOKMARKRECORD {
        QString sUUID;
        quint64 nLocation;
        XBinary::LT locationType;
        qint64 nSize;
        QColor colText;
        QColor colBackground;
        QString sTemplate;  // mb rename to sScript
        QString sComment;
    };
#endif

    bool isSymbolsPresent();                                   // TODO pdStruct
    QList<SYMBOL> getAllSymbols();                             // TODO pdStruct
    QMap<quint32, QString> getSymbolModules();                 // TODO pdStruct
    QList<REFERENCE> getReferencesForAddress(XADDR nAddress);  // TODO pdStruct

    //    QList<XADDR> getSymbolAddresses(ST symbolType);

    bool _addSymbol(XADDR nAddress, quint32 nModule, const QString &sSymbol, SS symSource);
    void _sortSymbols();
    qint32 _getSymbolIndex(XADDR nAddress, qint64 nSize, quint32 nModule, qint32 *pnInsertIndex);
    bool _addExportSymbol(XADDR nAddress, const QString &sSymbol);
    bool _addImportSymbol(XADDR nAddress, const QString &sSymbol);
    bool _addTLSSymbol(XADDR nAddress, const QString &sSymbol);

    //    static QString symbolSourceIdToString(SS symbolSource);
    //    static QString symbolTypeIdToString(ST symbolType);

    SYMBOL getSymbolByAddress(XADDR nAddress);
    bool isSymbolPresent(XADDR nAddress);
    QList<FUNCTION> getAllFunctions();
    bool isFunctionPresent(XADDR nAddress);
    QString getSymbolStringByAddress(XADDR nAddress);
    void initDisasmDb();
    void initHexDb();
#ifdef QT_SQL_LIB
    bool isTablePresent(QSqlDatabase *pDatabase, DBTABLE dbTable);
    bool isTableNotEmpty(QSqlDatabase *pDatabase, QString sTable);
    bool isTableNotEmpty(QSqlDatabase *pDatabase, DBTABLE dbTable);
    bool isTablePresentAndNotEmpty(QSqlDatabase *pDatabase, DBTABLE dbTable);
    void createTable(QSqlDatabase *pDatabase, DBTABLE dbTable);
    void removeTable(QSqlDatabase *pDatabase, QString sTable);
    void removeTable(QSqlDatabase *pDatabase, DBTABLE dbTable);
    void clearTable(QSqlDatabase *pDatabase, DBTABLE dbTable);
    QString getCreateSqlString(QSqlDatabase *pDatabase, QString sTable);
    QList<QString> getNotEmptyTables(QSqlDatabase *pDatabase);
#endif
    bool isDbPresent();
    void removeAllTables();
    void clearAllTables();
    void clearDb();
    void vacuumDb();
    void _addSymbolsFromFile(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1, XBinary::FT fileType = XBinary::FT_UNKNOWN,
                             XBinary::PDSTRUCT *pPdStruct = nullptr);
    void _addELFSymbols(XELF *pELF, XBinary::_MEMORY_MAP *pMemoryMap, qint64 nDataOffset, qint64 nDataSize, qint64 nStringsTableOffset, qint64 nStringsTableSize,
                        XBinary::PDSTRUCT *pPdStruct);

    struct ANALYZEOPTIONS {
        QIODevice *pDevice;
        XBinary::_MEMORY_MAP *pMemoryMap;
        XADDR nStartAddress;
        quint32 nCount;
        bool bAll;
    };

    bool _analyzeCode(const ANALYZEOPTIONS &analyzeOptions, XBinary::PDSTRUCT *pPdStruct = nullptr);
    bool _addShowRecord(const SHOWRECORD &record);
    bool _addRelRecord(const RELRECORD &record);
    void _completeDbAnalyze();
#ifdef QT_SQL_LIB
    bool _addShowRecord_prepare(QSqlQuery *pQuery);
    bool _addShowRecord_bind(QSqlQuery *pQuery, const SHOWRECORD &record);
    bool _isShowRecordPresent_prepare1(QSqlQuery *pQuery);
    bool _isShowRecordPresent_prepare2(QSqlQuery *pQuery);
    bool _isShowRecordPresent_bind1(QSqlQuery *pQuery, XADDR nAddress);
    bool _isShowRecordPresent_bind(QSqlQuery *pQuery1, QSqlQuery *pQuery2, XADDR nAddress, qint64 nSize);
    bool _addRelRecord_prepare(QSqlQuery *pQuery);
    bool _addRelRecord_bind(QSqlQuery *pQuery, const RELRECORD &record);
    bool _isShowRecordPresent(QSqlQuery *pQuery, XADDR nAddress, qint64 nSize);
    void _addShowRecords_bind(QSqlQuery *pQuery, QList<SHOWRECORD> *pListRecords);
    void _addRelRecords_bind(QSqlQuery *pQuery, QList<RELRECORD> *pListRecords);
    quint64 _getBranchNumber();
#endif
    QList<RELRECORD> getRelRecords(DBSTATUS dbstatus);
    bool _incShowRecordRefFrom(XADDR nAddress);
    bool _removeAnalyze(XADDR nAddress, qint64 nSize);
    void _clearAnalyze();
    bool _setArray(XADDR nAddress, qint64 nSize);
    bool _addFunction(XADDR nAddress, qint64 nSize, const QString &sName);
    void updateFunctionSize(XADDR nAddress, qint64 nSize);
#ifdef QT_GUI_LIB
    QString _addBookmarkRecord(const BOOKMARKRECORD &record);
    bool _removeBookmarkRecord(const QString &sUUID);
    QList<BOOKMARKRECORD> getBookmarkRecords(XBinary::PDSTRUCT *pPdStruct = nullptr);
    QList<BOOKMARKRECORD> getBookmarkRecords(quint64 nLocation, XBinary::LT locationType, qint64 nSize, XBinary::PDSTRUCT *pPdStruct = nullptr);
    void updateBookmarkRecord(BOOKMARKRECORD &record);
    void updateBookmarkRecordColor(const QString &sUUID, const QColor &colBackground);
    void updateBookmarkRecordComment(const QString &sUUID, const QString &sComment);
#endif

    SHOWRECORD getShowRecordByAddress(XADDR nAddress, bool bIsAprox);
    SHOWRECORD getNextShowRecordByAddress(XADDR nAddress);
    SHOWRECORD getPrevShowRecordByAddress(XADDR nAddress);
    SHOWRECORD getNextShowRecordByOffset(qint64 nOffset);
    SHOWRECORD getPrevShowRecordByOffset(qint64 nOffset);
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
    QList<SHOWRECORD> getShowRecords(qint64 nLine, qint32 nCount, XBinary::PDSTRUCT *pPdStruct);
    QList<SHOWRECORD> getShowRecordsInRegion(XADDR nAddress, qint64 nSize, XBinary::PDSTRUCT *pPdStruct = nullptr);
    QList<XADDR> getShowRecordRelAddresses(XCapstone::RELTYPE relType, DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct);
    QList<XBinary::ADDRESSSIZE> getShowRecordMemoryVariables(DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct);
    QList<XBinary::ADDRESSSIZE> getBranches(DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct);

    QList<XADDR> getExportSymbolAddresses(XBinary::PDSTRUCT *pPdStruct);
    QList<XADDR> getImportSymbolAddresses(XBinary::PDSTRUCT *pPdStruct);
    QList<XADDR> getTLSSymbolAddresses(XBinary::PDSTRUCT *pPdStruct);
    QList<XADDR> getFunctionAddresses(XBinary::PDSTRUCT *pPdStruct);

    RELRECORD getRelRecordByAddress(XADDR nAddress);
    bool isAddressHasRefFrom(XADDR nAddress);
    bool isAnalyzedRegionVirtual(XADDR nAddress, qint64 nSize);

    bool loadDbFromFile(const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct = nullptr);
    bool saveDbToFile(const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct = nullptr);
#ifdef USE_XPROCESS
    static QString threadStatusToString(THREAD_STATUS threadStatus);
#endif
#ifdef QT_SQL_LIB
    bool querySQL(QSqlQuery *pSqlQuery, const QString &sSQL, bool bWrite);
    bool querySQL(QSqlQuery *pSqlQuery, bool bWrite);
    QString convertStringSQLTableName(const QString &sSQL);
    QString convertStringSQLValue(const QString &sSQL);
    bool copyDb(QSqlDatabase *pDatabaseSource, QSqlDatabase *pDatabaseDest, XBinary::PDSTRUCT *pPdStruct);
#endif
    void testFunction();
    void setDebuggerState(bool bState);
    bool isDebugger();
#ifdef QT_GUI_LIB
    static QColor stringToColor(const QString &sCode);
    static QString colorToString(QColor color);
#endif
    QString convertOpcodeString(XCapstone::DISASM_RESULT disasmResult, XBinary::DM disasmMode, XBinary::SYNTAX syntax, const XInfoDB::RI_TYPE &riType,
                                const XCapstone::DISASM_OPTIONS &disasmOptions = XCapstone::DISASM_OPTIONS());
    QString _convertOpcodeString(const QString &sString, XADDR nAddress, XBinary::DM disasmMode, XBinary::SYNTAX syntax, const XInfoDB::RI_TYPE &riType,
                                 const XCapstone::DISASM_OPTIONS &disasmOptions);
    void setDatabaseChanged(bool bState);
    bool isDatabaseChanged();
public slots:
    void readDataSlot(quint64 nOffset, char *pData, qint64 nSize);
    void writeDataSlot(quint64 nOffset, char *pData, qint64 nSize);

private:
    void replaceMemory(quint64 nOffset, char *pData, qint64 nSize);

signals:
    void dataChanged(qint64 nDeviceOffset, qint64 nDeviceSize);
    void reloadSignal(bool bReloadData);  // TODO Check mb remove
    void reloadViewSignal();
    void memoryRegionsListChanged();
    void modulesListChanged();
    void threadsListChanged();
    void registersListChanged();
    //    void analyzeStateChanged();

private:
    struct _ENTRY {
        XADDR nAddress;
        quint32 nBranch;
    };
#ifdef USE_XPROCESS
    struct STATUS {
        quint32 nRegistersHash;
        QList<REG_RECORD> listRegs;
        QList<REG_RECORD> listRegsPrev;  // mb TODO move to prev
        X_ID nThreadId;
        X_HANDLE hThread;
        quint32 nMemoryRegionsHash;
        QList<XProcess::MEMORY_REGION> listMemoryRegions;  // TODO prev
        quint32 nModulesHash;
        QList<XProcess::MODULE> listModules;  // TODO prev
        quint32 nThreadsHash;
        QList<XProcess::THREAD_INFO> listThreads;  // TODO prev
    };
    XBinary::XVARIANT _getRegCache(QList<REG_RECORD> *pListRegs, XREG reg);
    void _setRegCache(QList<REG_RECORD> *pListRegs, XREG reg, XBinary::XVARIANT variant);
    void _addCurrentRegRecord(XREG reg, XBinary::XVARIANT value);
#endif
private:
#ifdef USE_XPROCESS
    XInfoDB::PROCESS_INFO g_processInfo;
    QList<BREAKPOINT> g_listBreakpoints;
    BPT g_bpTypeDefault;
#ifdef Q_OS_LINUX
    QMap<X_ID, BREAKPOINT> g_mapThreadBreakpoints;  // STEPS, ThreadID/BP TODO QList TODO remove
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
    QMutex *g_pMutexSQL;
    QMutex *g_pMutexThread;
#ifdef QT_SQL_LIB
    QString g_sDatabaseName;
    QSqlDatabase g_dataBase;
    QString s_sql_tableName[__DBTABLE_SIZE];
#endif
    bool g_bIsDatabaseChanged;
    bool g_bIsDebugger;
};

#endif  // XINFODB_H
