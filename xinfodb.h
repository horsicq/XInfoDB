/* Copyright (c) 2022-2025 hors<horsicq@gmail.com>
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
#include "xdisasmcore.h"
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
    enum SYMBOL_MODE {
        SYMBOL_MODE_UNKNOWN = 0,
        SYMBOL_MODE_ALL,
    };

    enum XSYMBOL_FLAG {
        XSYMBOL_FLAG_UNKNOWN = 0,
        XSYMBOL_FLAG_FUNCTION = 1 << 0,
        XSYMBOL_FLAG_ENTRYPOINT = 1 << 1,
        XSYMBOL_FLAG_EXPORT = 1 << 2,
        XSYMBOL_FLAG_IMPORT = 1 << 3,
        XSYMBOL_FLAG_ADDRESS = 1 << 4,
        XSYMBOL_FLAG_DATA = 1 << 5,
    };

    enum XRECORD_FLAG {
        XRECORD_FLAG_UNKNOWN = 0,
        XRECORD_FLAG_CODE = 1 << 0,
        XRECORD_FLAG_DATA = 1 << 1,
        XRECORD_FLAG_OPCODE = 1 << 2,
        XRECORD_FLAG_ARRAY = 1 << 3,
        XRECORD_FLAG_ADDRREF = 1 << 4,
    };

    enum XREF_FLAG {
        XREF_FLAG_UNKNOWN = 0,
        XREF_FLAG_REL = 1 << 1,
        XREF_FLAG_MEMORY = 1 << 2,
        XREF_FLAG_CALL = 1 << 3,
        XREF_FLAG_JMP = 1 << 4,
        XREF_FLAG_JMP_COND = 1 << 4,
        XREF_FLAG_RET = 1 << 5,
    };

    struct XRECORD {
        quint16 nRegionIndex;
        quint16 nFlags;
        quint16 nSize;
        quint16 nBranch;
        quint64 nRelOffset;
    };

    struct XREFINFO {
        quint16 nRegionIndex;
        quint16 nRegionIndexRef;
        quint32 nSize;
        quint64 nRelOffset;
        quint64 nRelOffsetRef;
        quint16 nFlags;
        quint16 nBranch;
    };

    struct XSYMBOL {
        quint16 nStringIndex;
        quint16 nBranch;
        quint16 nRegionIndex;
        quint16 nFlags;
        quint32 nSize;
        quint64 nRelOffset;
    };

    struct STATE {
        QIODevice *pDevice;
        XADDR nModuleAddress;
        bool bIsImage;
        XBinary::_MEMORY_MAP memoryMap;
        XDisasmCore disasmCore;
        QVector<XRECORD> listRecords;
        QVector<XREFINFO> listRefs;
        QVector<XSYMBOL> listSymbols;
        QVector<QString> listStrings;
        QSet<XADDR> stCodeTemp;
        quint16 nCurrentBranch;
        bool bIsAnalyzed;
    };

    struct BOOKMARKRECORD {
        QString sUUID;
        quint64 nLocation;
        XBinary::LT locationType;
        qint64 nSize;
        QString sColorText;
        QString sColorBackground;
        QString sTemplate;
        QString sComment;
        bool bIsUser;
    };

    struct USER_NOTES {
        quint32 nDummy;
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
        DBTABLE_SYMBOLS,
        DBTABLE_REFINFO,
        DBTABLE_RECORDS,
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
    void initDB();
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
    // XDisasmAbstract::DISASM_RESULT disasm(XADDR nAddress);
    qint64 read_userData(X_ID nThreadId, qint64 nOffset, char *pData, qint64 nSize);
    qint64 write_userData(X_ID nThreadId, qint64 nOffset, char *pData, qint64 nSize);
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

    // XCapstone::OPCODE_ID getCurrentOpcode_Handle(X_HANDLE hThread);
    // XCapstone::OPCODE_ID getCurrentOpcode_Id(X_ID nThreadId);

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

    bool isSymbolPresent(XADDR nAddress);
    bool isFunctionPresent(XADDR nAddress);
    QString getSymbolStringByAddress(XADDR nAddress);
#ifdef QT_SQL_LIB
    bool isTablePresent(QSqlDatabase *pDatabase, DBTABLE dbTable);
    bool isTableNotEmpty(QSqlDatabase *pDatabase, const QString &sTable);
    bool isTableNotEmpty(QSqlDatabase *pDatabase, DBTABLE dbTable);
    bool isTablePresentAndNotEmpty(QSqlDatabase *pDatabase, DBTABLE dbTable);
    void createTable(QSqlDatabase *pDatabase, DBTABLE dbTable);
    void removeTable(QSqlDatabase *pDatabase, const QString &sTable);
    void removeTable(QSqlDatabase *pDatabase, DBTABLE dbTable);
    void clearTable(QSqlDatabase *pDatabase, DBTABLE dbTable);
    QString getCreateSqlString(QSqlDatabase *pDatabase, const QString &sTable);
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
    bool _analyze(XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct);
    void _addCode(STATE *pState, XBinary::_MEMORY_RECORD *pMemoryRecord, char *pMemory, XADDR nRelOffset, qint64 nSize, quint16 nBranch, XBinary::PDSTRUCT *pPdStruct);
    bool _isCode(STATE *pState, XBinary::_MEMORY_RECORD *pMemoryRecord, char *pMemory, XADDR nRelOffset, qint64 nSize);
    bool addSymbol(STATE *pState, XADDR nAddress, quint32 nSize, quint16 nFlags, const QString &sSymbolName = QString(), quint16 nBranch = 0);
    bool addRefInfo(STATE *pState, XADDR nAddress, XADDR nAddressRef, quint32 nSize, quint16 nFlags, quint16 nBranch = 0);
    bool addRecord(STATE *pState, XADDR nAddress, quint16 nSize, quint16 nFlags, quint16 nBranch = 0);
    bool updateSymbolFlags(STATE *pState, XADDR nAddress, quint16 nFlags);
    bool addSymbolOrUpdateFlags(STATE *pState, XADDR nAddress, quint32 nSize, quint16 nFlags, const QString &sSymbolName = QString());
    void dumpBookmarks();
    void dumpSymbols(XBinary::FT fileType);
    void dumpRecords(XBinary::FT fileType);
    void dumpRefs(XBinary::FT fileType);
    void dumpShowRecords(XBinary::FT fileType);

    QString getShowString(STATE *pState, const XRECORD &record, const XDisasmAbstract::DISASM_OPTIONS &disasmOptions);
    QString _getSymbolStringBySegmentRelOffset(STATE *pState, quint16 nRegionIndex, XADDR nRelOffset);
    QString _getSymbolStringByAddress(STATE *pState, XADDR nAddress);

    void setData(QIODevice *pDevice, XBinary::FT fileType);
    XBinary::FT addMode(QIODevice *pDevice, XBinary::FT fileType);

    qint32 _searchXRecordBySegmentRelOffset(QVector<XRECORD> *pListRecords, quint16 nRegionIndex, XADDR nRelOffset, bool bInRecord);
    bool _insertXRecord(QVector<XRECORD> *pListSymbols, const XRECORD &record);
    qint32 _searchXRecordByAddress(XBinary::_MEMORY_MAP *pMemoryMap, QVector<XRECORD> *pListRecords, XADDR nAddress, bool bInRecord);
    qint32 _searchXRecordByAddress(STATE *pState, XADDR nAddress, bool bInRecord);

    qint32 _searchXSymbolByAddress(XBinary::_MEMORY_MAP *pMemoryMap, QVector<XSYMBOL> *pListSymbols, XADDR nAddress);
    qint32 _searchXSymbolBySegmentRelOffset(QVector<XSYMBOL> *pListSymbols, quint16 nRegionIndex, XADDR nRelOffset);
    bool _insertXSymbol(QVector<XSYMBOL> *pListSymbols, const XSYMBOL &symbol);

    qint32 _searchXRefinfoBySegmentRelOffset(QVector<XREFINFO> *pListRefs, quint16 nRegionIndex, XADDR nRelOffset);
    bool _insertXRefinfo(QVector<XREFINFO> *pListRefs, const XREFINFO &refinfo);

    static qint64 getOffset(STATE *pState, quint16 nRegionIndex, XADDR nRelOffset);
    static XADDR getAddress(STATE *pState, quint16 nRegionIndex, XADDR nRelOffset);

    void _completeDbAnalyze();
#ifdef QT_SQL_LIB
    bool _addShowRecord_prepare(QSqlQuery *pQuery);
    bool _isShowRecordPresent_prepare1(QSqlQuery *pQuery);
    bool _isShowRecordPresent_prepare2(QSqlQuery *pQuery);
    bool _isShowRecordPresent_bind1(QSqlQuery *pQuery, XADDR nAddress);
    bool _isShowRecordPresent_bind(QSqlQuery *pQuery1, QSqlQuery *pQuery2, XADDR nAddress, qint64 nSize);
    bool _addRelRecord_prepare(QSqlQuery *pQuery);
    bool _isShowRecordPresent(QSqlQuery *pQuery, XADDR nAddress, qint64 nSize);
    quint64 _getBranchNumber();
#endif
    bool _incShowRecordRefFrom(XADDR nAddress);
    bool _removeAnalyze(XADDR nAddress, qint64 nSize);
    void _clearAnalyze();
    bool _setArray(XADDR nAddress, qint64 nSize);
    bool _addFunction(XADDR nAddress, qint64 nSize, const QString &sName);
    void updateFunctionSize(XADDR nAddress, qint64 nSize);
    QString _addBookmarkRecord(const BOOKMARKRECORD &record);
    bool _removeBookmarkRecord(const QString &sUUID);
    QVector<BOOKMARKRECORD> *getBookmarkRecords();
    QVector<BOOKMARKRECORD> getBookmarkRecords(quint64 nLocation, XBinary::LT locationType, qint64 nSize, XBinary::PDSTRUCT *pPdStruct = nullptr);
    QString addBookmarkRecord(const BOOKMARKRECORD &record);
    bool removeBookmarkRecord(const QString &sUUID);
    void updateBookmarkRecord(BOOKMARKRECORD &record);
    void updateBookmarkRecordColorBackground(const QString &sUUID, const QString &sColorBackground);
    void updateBookmarkRecordComment(const QString &sUUID, const QString &sComment);

    // XInfoDB::XRECORD getRecordByAddress(XBinary::FT fileType, XADDR nAddress, bool bInRecord);
    // XADDR segmentRelOffsetToAddress(XBinary::FT fileType, quint16 nSegment, XADDR nRelOffset);

    qint64 getShowRecordOffsetByAddress(XADDR nAddress);
    qint64 getShowRecordPrevOffsetByAddress(XADDR nAddress);
    qint64 getShowRecordOffsetByLine(qint64 nLine);
    XADDR getShowRecordAddressByOffset(qint64 nOffset);
    XADDR getShowRecordAddressByLine(qint64 nLine);
    qint64 getRecordsCount(XBinary::FT fileType);
    qint64 getShowRecordLineByAddress(XADDR nAddress);
    qint64 getShowRecordLineByOffset(qint64 nOffset);
    void updateShowRecordLine(XADDR nAddress, qint64 nLine);
    QList<XADDR> getShowRecordRelAddresses(XDisasmAbstract::RELTYPE relType, DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct);
    QList<XBinary::ADDRESSSIZE> getShowRecordMemoryVariables(DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct);
    QList<XBinary::ADDRESSSIZE> getBranches(DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct);

    QList<XADDR> getExportSymbolAddresses(XBinary::PDSTRUCT *pPdStruct);
    QList<XADDR> getImportSymbolAddresses(XBinary::PDSTRUCT *pPdStruct);
    QList<XADDR> getTLSSymbolAddresses(XBinary::PDSTRUCT *pPdStruct);
    QList<XADDR> getFunctionAddresses(XBinary::PDSTRUCT *pPdStruct);

    bool isAddressHasRefFrom(XADDR nAddress);
    bool isAnalyzedRegionVirtual(XADDR nAddress, qint64 nSize);

    bool loadDbFromFile(QIODevice *pDevice, const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct = nullptr);
    bool saveDbToFile(const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct = nullptr);
#ifdef USE_XPROCESS
    static QString threadStatusToString(THREAD_STATUS threadStatus);
#endif
#ifdef QT_SQL_LIB
    bool querySQL(QSqlQuery *pSqlQuery, const QString &sSQL, bool bWrite);
    bool querySQL(QSqlQuery *pSqlQuery, bool bWrite);
    QString convertStringSQLTableName(const QString &sSQL);
    QString convertStringSQLValue(const QString &sSQL);
#endif
    void testFunction();
    void setDebuggerState(bool bState);
    bool isDebugger();
    QString convertOpcodeString(XDisasmAbstract::DISASM_RESULT disasmResult, const XInfoDB::RI_TYPE &riType, const XDisasmAbstract::DISASM_OPTIONS &disasmOptions);
    QString _convertOpcodeString(const QString &sString, XADDR nAddress, const XInfoDB::RI_TYPE &riType, const XDisasmAbstract::DISASM_OPTIONS &disasmOptions);
    void setDatabaseChanged(bool bState);
    bool isDatabaseChanged();

    STATE *getState(XBinary::FT fileType);
    bool isAnalyzed(XBinary::FT fileType);
    bool isStatePresent(XBinary::FT fileType);

public slots:
    void readDataSlot(quint64 nOffset, char *pData, qint64 nSize);
    void writeDataSlot(quint64 nOffset, char *pData, qint64 nSize);

private:
    void replaceMemory(quint64 nOffset, char *pData, qint64 nSize);

signals:
    void dataChanged(qint64 nDeviceOffset, qint64 nDeviceSize);
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
    XInfoDB::PROCESS_INFO m_processInfo;
    QList<BREAKPOINT> m_listBreakpoints;
    BPT m_bpTypeDefault;
#ifdef Q_OS_LINUX
    QMap<X_ID, BREAKPOINT> m_mapThreadBreakpoints;  // STEPS, ThreadID/BP TODO QList TODO remove
#endif
    //    QMap<X_ID,BREAKPOINT> g_mapThreadBreakpoints;         // STEPS,
    //    ThreadID/BP TODO QList
    QMap<XADDR, SHAREDOBJECT_INFO> m_mapSharedObjectInfos;  // TODO QList
    QList<THREAD_INFO> m_listThreadInfos;
    QMap<QString, FUNCTIONHOOK_INFO> m_mapFunctionHookInfos;  // TODO QList
#endif
    // MODE m_mode;  // TODO remove
    // MODE g_defaultMode;
#ifdef USE_XPROCESS
    STATUS m_statusCurrent;
//    STATUS g_statusPrev;
#endif
    QMutex *m_pMutexSQL;
    QMutex *m_pMutexThread;
#ifdef QT_SQL_LIB
    QString s_sql_tableName[__DBTABLE_SIZE];
#endif
    bool m_bIsDatabaseChanged;
    bool m_bIsDebugger;
    QMap<XBinary::FT, STATE *> m_mapProfiles;
    QVector<BOOKMARKRECORD> m_listBookmarks;
};

#endif  // XINFODB_H
