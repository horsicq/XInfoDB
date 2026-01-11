/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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
#include "xinfodb.h"

bool compareXRECORD_location(const XInfoDB::XRECORD &a, const XInfoDB::XRECORD &b)
{
    if (a.nRegionIndex != b.nRegionIndex) {
        return a.nRegionIndex < b.nRegionIndex;
    } else {
        return a.nRelOffset < b.nRelOffset;
    }
}

bool compareXREFINFO_location(const XInfoDB::XREFINFO &a, const XInfoDB::XREFINFO &b)
{
    if (a.nRegionIndex != b.nRegionIndex) {
        return a.nRegionIndex < b.nRegionIndex;
    } else {
        return a.nRelOffset < b.nRelOffset;
    }
}

bool compareXREFINFO_location_ref(const XInfoDB::XREFINFO &a, const XInfoDB::XREFINFO &b)
{
    if (a.nRegionIndexRef != b.nRegionIndexRef) {
        return a.nRegionIndexRef < b.nRegionIndexRef;
    } else {
        return a.nRelOffsetRef < b.nRelOffsetRef;
    }
}

bool compareXSYMBOL_location(const XInfoDB::XSYMBOL &a, const XInfoDB::XSYMBOL &b)
{
    if (a.nRegionIndex != b.nRegionIndex) {
        return a.nRegionIndex < b.nRegionIndex;
    } else {
        return a.nRelOffset < b.nRelOffset;
    }
}

XInfoDB::XInfoDB(QObject *pParent) : QObject(pParent)
{
#ifdef USE_XPROCESS
    m_processInfo = {};

    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_INT1); // Checked Win
    setDefaultBreakpointType(BPT_CODE_SOFTWARE_INT3);  // Checked Win
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_HLT); // Checked Win
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_CLI); // Checked Win
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_STI);
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_INSB);
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_INSD);
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_OUTSB);
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_OUTSD);
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_INT1LONG);  // Checked Win
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_INT3LONG); // Checked Win
    //    setDefaultBreakpointType(BPT_CODE_SOFTWARE_UD0);
    //    setDefaultBreakpointType( BPT_CODE_SOFTWARE_UD2);
#endif
    m_bIsDebugger = false;
    m_pMutexSQL = new QMutex;
    m_pMutexThread = new QMutex;
    m_bIsDatabaseChanged = false;
}

XInfoDB::~XInfoDB()
{
    clearDb();
#ifdef QT_SQL_LIB
#ifdef QT_DEBUG
    qDebug("XInfoDB::~XInfoDB()");
#endif
    // if (g_dataBase.isOpen()) {
    //     g_dataBase.close();
    //     g_dataBase = QSqlDatabase();
    //     QSqlDatabase::removeDatabase(g_sDatabaseName);
    // }
#endif

    QList<STATE *> listStates = m_mapProfiles.values();
    qint32 nNumberOfRecords = listStates.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        delete listStates.at(i);
    }

    delete m_pMutexSQL;
    delete m_pMutexThread;
}

void XInfoDB::initDB()
{
    // #ifdef QT_SQL_LIB
    //     if (!g_dataBase.isOpen()) {
    //         g_sDatabaseName = QString("memdb_%1").arg(XBinary::randomString(10));
    //         g_dataBase = QSqlDatabase::addDatabase("QSQLITE", g_sDatabaseName);

    //         //        g_dataBase.setDatabaseName(":memory:");
    // #ifdef Q_OS_WIN
    //         g_dataBase.setDatabaseName(":memory:");
    //         //        g_dataBase.setDatabaseName("C:\\tmp_build\\local_dbXS.db");
    // #else
    // #ifndef QT_DEBUG
    //         g_dataBase.setDatabaseName(":memory:");
    // #endif
    //         // g_dataBase.setDatabaseName(":memory:");
    //         g_dataBase.setDatabaseName("/home/hors/local_db.db");
    // #endif
    //         // #ifdef Q_OS_LINUX
    //         //     g_dataBase.setDatabaseName("/home/hors/local_db.db");
    //         // #endif
    //         //  #ifndef QT_DEBUG
    //         //      g_dataBase.setDatabaseName(":memory:");
    //         //  #else
    //         //  #ifdef Q_OS_WIN
    //         //      g_dataBase.setDatabaseName("C:\\tmp_build\\local_dbXS.db");
    //         ////    g_dataBase.setDatabaseName(":memory:");
    //         // #else
    //         //     g_dataBase.setDatabaseName(":memory:");
    //         // #endif
    //         ////    g_dataBase.setDatabaseName(":memory:");
    //         // #endif

    //         if (g_dataBase.open()) {
    //             g_dataBase.exec("PRAGMA synchronous = OFF");
    //             g_dataBase.exec("PRAGMA journal_mode = MEMORY");

    //             // setAnalyzed(isSymbolsPresent());
    //         } else {
    // #ifdef QT_DEBUG
    //             qDebug("Cannot open sqlite database");
    // #endif
    //         }
    //     }
    // #endif
}

void XInfoDB::reloadView()
{
    emit reloadViewSignal();
}

void XInfoDB::setEdited(qint64 nDeviceOffset, qint64 nDeviceSize)
{
    Q_UNUSED(nDeviceOffset)
    Q_UNUSED(nDeviceSize)
    // TODO
}

void XInfoDB::_createTableNames(){
#ifdef QT_SQL_LIB
// QString sPrefix = XBinary::fileTypeIdToString(g_fileType);
// s_sql_tableName[DBTABLE_SYMBOLS] = convertStringSQLTableName(QString("%1_SYMBOLS").arg(sPrefix));
// s_sql_tableName[DBTABLE_SHOWRECORDS] = convertStringSQLTableName(QString("%1_SHOWRECORDS").arg(sPrefix));
// s_sql_tableName[DBTABLE_RELATIVS] = convertStringSQLTableName(QString("%1_RELRECORDS").arg(sPrefix));
// s_sql_tableName[DBTABLE_IMPORT] = convertStringSQLTableName(QString("%1_IMPORT").arg(sPrefix));
// s_sql_tableName[DBTABLE_EXPORT] = convertStringSQLTableName(QString("%1_EXPORT").arg(sPrefix));
// s_sql_tableName[DBTABLE_TLS] = convertStringSQLTableName(QString("%1_TLS").arg(sPrefix));
// s_sql_tableName[DBTABLE_FUNCTIONS] = convertStringSQLTableName(QString("%1_FUNCTIONS").arg(sPrefix));
// s_sql_tableName[DBTABLE_BOOKMARKS] = convertStringSQLTableName(QString("BOOKMARKS"));
#endif
}

#ifdef USE_XPROCESS
quint32 XInfoDB::read_uint32(XADDR nAddress, bool bIsBigEndian)
{
    quint32 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::read_uint32(m_processInfo.hProcess, nAddress, bIsBigEndian);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::read_uint32(m_processInfo.hProcessMemoryIO, nAddress, bIsBigEndian);
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
quint64 XInfoDB::read_uint64(XADDR nAddress, bool bIsBigEndian)
{
    quint64 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::read_uint64(m_processInfo.hProcess, nAddress, bIsBigEndian);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::read_uint64(m_processInfo.hProcessMemoryIO, nAddress, bIsBigEndian);
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
qint64 XInfoDB::read_array(XADDR nAddress, char *pData, quint64 nSize)
{
    qint64 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::read_array(m_processInfo.hProcess, nAddress, pData, nSize);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::read_array(m_processInfo.hProcessMemoryIO, nAddress, pData, nSize);
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
qint64 XInfoDB::write_array(XADDR nAddress, char *pData, quint64 nSize)
{
    qint64 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::write_array(m_processInfo.hProcess, nAddress, pData, nSize);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::write_array(m_processInfo.hProcessMemoryIO, nAddress, pData, nSize);
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
QByteArray XInfoDB::read_array(XADDR nAddress, quint64 nSize)
{
    QByteArray baResult;
#ifdef Q_OS_WIN
    baResult = XProcess::read_array(m_processInfo.hProcess, nAddress, nSize);
#endif
#ifdef Q_OS_LINUX
    baResult = XProcess::read_array(m_processInfo.hProcessMemoryIO, nAddress, nSize);
#endif
    return baResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::read_ansiString(XADDR nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef Q_OS_WIN
    sResult = XProcess::read_ansiString(m_processInfo.hProcess, nAddress, nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult = XProcess::read_ansiString(m_processInfo.hProcessMemoryIO, nAddress, nMaxSize);
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::read_unicodeString(XADDR nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef Q_OS_WIN
    sResult = XProcess::read_unicodeString(m_processInfo.hProcess, nAddress, nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult = XProcess::read_unicodeString(m_processInfo.hProcessMemoryIO, nAddress, nMaxSize);
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::read_utf8String(XADDR nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef Q_OS_WIN
    sResult = XProcess::read_utf8String(m_processInfo.hProcess, nAddress, nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult = XProcess::read_utf8String(m_processInfo.hProcessMemoryIO, nAddress, nMaxSize);
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
// XDisasmAbstract::DISASM_RESULT XInfoDB::disasm(XADDR nAddress)
// {
//     QByteArray baArray = read_array(nAddress, 16);

//     return XCapstone::disasm_ex(g_handle, getDisasmMode(), XBinary::SYNTAX_DEFAULT, baArray.data(), baArray.size(), nAddress);
// }
#endif
#ifdef USE_XPROCESS
qint64 XInfoDB::read_userData(X_ID nThreadId, qint64 nOffset, char *pData, qint64 nSize)
{
    qint64 nResult = 0;
#ifdef Q_OS_LINUX
    qint32 nDelta = sizeof(unsigned long);

    for (qint32 i = 0; i < nSize; i += nDelta) {
        if (nDelta == 4) {
            *((quint32 *)(pData + i)) = ptrace(PTRACE_PEEKUSER, nThreadId, nOffset + i, 0);
        } else if (nDelta == 8) {
            *((quint64 *)(pData + i)) = ptrace(PTRACE_PEEKUSER, nThreadId, nOffset + i, 0);
        }
    }

    nResult = nSize;
#else
    Q_UNUSED(nThreadId)
    Q_UNUSED(nOffset)
    Q_UNUSED(pData)
    Q_UNUSED(nSize)
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
qint64 XInfoDB::write_userData(X_ID nThreadId, qint64 nOffset, char *pData, qint64 nSize)
{
    qint64 nResult = 0;
#ifdef Q_OS_LINUX
    qint32 nDelta = sizeof(unsigned long);

    for (qint32 i = 0; i < nSize; i += nDelta) {
        if (nDelta == 4) {
            ptrace(PTRACE_POKEUSER, nThreadId, nOffset + i, *((quint32 *)(pData + i)));
        } else if (nDelta == 8) {
            ptrace(PTRACE_POKEUSER, nThreadId, nOffset + i, *((quint64 *)(pData + i)));
        }
    }

    nResult = nSize;
#else
    Q_UNUSED(nThreadId)
    Q_UNUSED(nOffset)
    Q_UNUSED(pData)
    Q_UNUSED(nSize)
#endif
    return nResult;
}
#endif

QList<QString> XInfoDB::getStringsFromFile(const QString &sFileName, XBinary::PDSTRUCT *pPdStruct)
{
    QList<QString> listResult;

    QFile inputFile(sFileName);

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        while ((!in.atEnd()) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            QString sLine = in.readLine();

            listResult.append(sLine);
        }
        inputFile.close();
    }

    return listResult;
}

XInfoDB::STRRECORD XInfoDB::handleStringDB(QList<QString> *pListStrings, STRDB strDB, const QString &sString, bool bIsMulti, XBinary::PDSTRUCT *pPdStruct)
{
    STRRECORD result = {};

    qint32 nNumberOfRecords = pListStrings->count();

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        QString sRecord = pListStrings->at(i);

        if ((strDB == STRDB_PESECTIONS) || (strDB == STRDB_ELFSECTIONS)) {
            if (sRecord.contains("|")) {
                QString sValue = sRecord.section("|", 0, -3);

                if (sString == sValue) {
                    QString sType = sRecord.section("|", -2, -2);
                    QString _sString = sRecord.section("|", -1, -1);

                    if (result.sDescription != "") {
                        result.sDescription += " | ";
                    }

                    if (sType != "") {
                        result.sDescription += QString("(%1) ").arg(XScanEngine::translateType(sType));
                    }

                    result.sDescription += _sString;

                    if (result.sString == "") {
                        result.sString = _sString;
                        result.sType = sType;
                    }

                    if (!bIsMulti) {
                        break;
                    }
                }
            }
        } else if (strDB == STRDB_LIBRARIES) {
            // TODO
        } else if (strDB == STRDB_FUNCTIONS) {
            // TODO
        }
    }

    return result;
}

QList<QString> XInfoDB::loadStrDB(const QString &sPath, STRDB strDB, XBinary::PDSTRUCT *pPdStruct)
{
    QList<QString> listResult;

    QString sStrDBFileName;

    if (strDB == STRDB_PESECTIONS) {
        sStrDBFileName = "PE.sections.txt";
    }

    if (sStrDBFileName != "") {
        listResult = getStringsFromFile(XBinary::convertPathName(sPath) + QDir::separator() + sStrDBFileName, pPdStruct);
    }

    return listResult;
}
#ifdef USE_XPROCESS
void XInfoDB::setDefaultBreakpointType(BPT bpType)
{
    m_bpTypeDefault = bpType;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentThreadId(X_ID nThreadId)
{
    m_statusCurrent.nThreadId = nThreadId;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentThreadHandle(X_HANDLE hThread)
{
    m_statusCurrent.hThread = hThread;
}
#endif
#ifdef USE_XPROCESS
X_ID XInfoDB::getCurrentThreadId()
{
    return m_statusCurrent.nThreadId;
}
#endif
#ifdef USE_XPROCESS
X_HANDLE XInfoDB::getCurrentThreadHandle()
{
    return m_statusCurrent.hThread;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::stepOver_Handle(X_HANDLE hThread, BPI bpInfo)
{
    bool bResult = false;

    XADDR nAddress = getCurrentInstructionPointer_Handle(hThread);
    XADDR nNextAddress = getAddressNextInstructionAfterCall(nAddress);  // TODO rep

    if (nNextAddress != (XADDR)-1) {
        XInfoDB::BREAKPOINT breakPoint = {};
        breakPoint.nAddress = nNextAddress;
        breakPoint.bpType = XInfoDB::BPT_CODE_SOFTWARE_DEFAULT;
        breakPoint.bpInfo = bpInfo;
        breakPoint.bOneShot = true;

        bResult = addBreakPoint(breakPoint);
    } else {
        bResult = stepInto_Handle(hThread, bpInfo);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::stepOver_Id(X_ID nThreadId, BPI bpInfo)
{
    bool bResult = false;

    XADDR nAddress = getCurrentInstructionPointer_Id(nThreadId);
    XADDR nNextAddress = getAddressNextInstructionAfterCall(nAddress);  // TODO rep

    if (nNextAddress != (XADDR)-1) {
        XInfoDB::BREAKPOINT breakPoint = {};
        breakPoint.nAddress = nNextAddress;
        breakPoint.bpType = XInfoDB::BPT_CODE_SOFTWARE_DEFAULT;
        breakPoint.bpInfo = bpInfo;

        bResult = addBreakPoint(breakPoint);
    } else {
        bResult = stepInto_Id(nThreadId, bpInfo);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByAddress(XADDR nAddress, BPT bpType)
{
    if (bpType == BPT_CODE_SOFTWARE_DEFAULT) {
        bpType = m_bpTypeDefault;
    }

    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if ((m_listBreakpoints.at(i).nAddress == nAddress) && (m_listBreakpoints.at(i).bpType == bpType)) {
            result = m_listBreakpoints.at(i);

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByExceptionAddress(XADDR nExceptionAddress, BPT bpType)
{
    if (bpType == BPT_CODE_SOFTWARE_DEFAULT) {
        bpType = m_bpTypeDefault;
    }

    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XInfoDB::BREAKPOINT breakPoint = m_listBreakpoints.at(i);

        if ((breakPoint.nAddress == (nExceptionAddress - breakPoint.nDataSize)) && (m_listBreakpoints.at(i).bpType == bpType)) {
            result = breakPoint;

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByThreadID(X_ID nThreadID, BPT bpType)
{
    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XInfoDB::BREAKPOINT breakPoint = m_listBreakpoints.at(i);

        if ((breakPoint.nThreadID == nThreadID) && (m_listBreakpoints.at(i).bpType == bpType)) {
            result = breakPoint;

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByUUID(const QString &sUUID)
{
    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (m_listBreakpoints.at(i).sUUID == sUUID) {
            result = m_listBreakpoints.at(i);

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByRegion(XADDR nAddress, qint64 nSize)
{
    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (XBinary::_isAddressCrossed(m_listBreakpoints.at(i).nAddress, m_listBreakpoints.at(i).nDataSize, nAddress, nSize)) {
            result = m_listBreakpoints.at(i);

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
qint32 XInfoDB::getThreadBreakpointsCount(X_ID nThreadID)
{
    qint32 nResult = 0;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (m_listBreakpoints.at(i).nThreadID == nThreadID) {
            nResult++;
        }
    }

    return nResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::breakpointToggle(XADDR nAddress)
{
    bool bResult = false;

    BREAKPOINT bp = findBreakPointByRegion(nAddress, 1);

    if (bp.bpInfo == XInfoDB::BPI_TOGGLE) {
        if (removeBreakPoint(bp.sUUID)) {
            bResult = true;
        }
    } else {
        XInfoDB::BREAKPOINT breakPoint = {};
        breakPoint.nAddress = nAddress;
        breakPoint.bpType = XInfoDB::BPT_CODE_SOFTWARE_DEFAULT;
        breakPoint.bpInfo = XInfoDB::BPI_TOGGLE;

        if (addBreakPoint(breakPoint)) {
            bResult = true;
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::breakpointRemove(XADDR nAddress)
{
    bool bResult = false;

    BREAKPOINT bp = findBreakPointByRegion(nAddress, 1);

    if (bp.sUUID != "") {
        if (removeBreakPoint(bp.sUUID)) {
            bResult = true;
        }
    }
    return bResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::bptToString(BPT bpType)
{
    QString sResult = tr("Unknown");
#ifdef Q_PROCESSOR_X86
    if (bpType == BPT_CODE_SOFTWARE_INT1) sResult = QString("INT1(0xF1)");
    else if (bpType == BPT_CODE_SOFTWARE_INT3) sResult = QString("INT3(0xCC)");
    else if (bpType == BPT_CODE_SOFTWARE_HLT) sResult = QString("HLT(0xF4)");
    else if (bpType == BPT_CODE_SOFTWARE_CLI) sResult = QString("CLI(0xFA)");
    else if (bpType == BPT_CODE_SOFTWARE_STI) sResult = QString("STI");
    else if (bpType == BPT_CODE_SOFTWARE_INSB) sResult = QString("INSB");
    else if (bpType == BPT_CODE_SOFTWARE_INSD) sResult = QString("INSD");
    else if (bpType == BPT_CODE_SOFTWARE_OUTSB) sResult = QString("OUTSD");
    else if (bpType == BPT_CODE_SOFTWARE_OUTSD) sResult = QString("OUTSD");
    else if (bpType == BPT_CODE_SOFTWARE_INT1LONG) sResult = QString("INT1 LONG (2 bytes)");
    else if (bpType == BPT_CODE_SOFTWARE_INT3LONG) sResult = QString("INT3 LONG (2 bytes)");
    else if (bpType == BPT_CODE_SOFTWARE_UD0) sResult = QString("UD0 (2 bytes)");
    else if (bpType == BPT_CODE_SOFTWARE_UD2) sResult = QString("UD2 (2 bytes)");
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::bpiToString(BPI bpInfo)
{
    QString sResult = tr("Unknown");
#ifdef Q_PROCESSOR_X86
    if (bpInfo == BPI_FUNCTIONENTER) sResult = tr("Function enter");
    else if (bpInfo == BPI_FUNCTIONLEAVE) sResult = tr("Function leave");
    else if (bpInfo == BPI_STEPINTO) sResult = tr("Step into");
    else if (bpInfo == BPI_STEPOVER) sResult = tr("Step over");
    else if (bpInfo == BPI_TRACEINTO) sResult = tr("Trace into");
    else if (bpInfo == BPI_TRACEOVER) sResult = tr("Trace over");
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::addSharedObjectInfo(SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    m_mapSharedObjectInfos.insert(pSharedObjectInfo->nImageBase, *pSharedObjectInfo);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::removeSharedObjectInfo(SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    m_mapSharedObjectInfos.remove(pSharedObjectInfo->nImageBase);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::addThreadInfo(THREAD_INFO *pThreadInfo)
{
    m_pMutexThread->lock();
    m_listThreadInfos.append(*pThreadInfo);
    m_pMutexThread->unlock();
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::removeThreadInfo(X_ID nThreadID)
{
    m_pMutexThread->lock();

    qint32 nNumberOfThread = m_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfThread; i++) {
        if (m_listThreadInfos.at(i).nThreadID == nThreadID) {
            m_listThreadInfos.removeAt(i);

            break;
        }
    }

    m_pMutexThread->unlock();
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setThreadStatus(X_ID nThreadID, THREAD_STATUS status)
{
    m_pMutexThread->lock();

    bool bResult = false;
    qint32 nNumberOfThread = m_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfThread; i++) {
        if (m_listThreadInfos.at(i).nThreadID == nThreadID) {
            m_listThreadInfos[i].threadStatus = status;

            break;
        }
    }

    m_pMutexThread->unlock();

    return bResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::THREAD_STATUS XInfoDB::getThreadStatus(X_ID nThreadID)
{
    m_pMutexThread->lock();

    THREAD_STATUS result = THREAD_STATUS_UNKNOWN;
    qint32 nNumberOfThread = m_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfThread; i++) {
        if (m_listThreadInfos.at(i).nThreadID == nThreadID) {
            result = m_listThreadInfos[i].threadStatus;

            break;
        }
    }

    m_pMutexThread->unlock();

    return result;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setFunctionHook(const QString &sFunctionName)
{
    Q_UNUSED(sFunctionName)

    bool bResult = false;

    //    qint64 nFunctionAddress = getFunctionAddress(sFunctionName);

    //    if (nFunctionAddress != -1) {
    //        bResult = addBreakPoint(nFunctionAddress, XInfoDB::BPT_CODE_SOFTWARE_DEFAULT, XInfoDB::BPI_FUNCTIONENTER, -1, sFunctionName);

    //        XInfoDB::FUNCTIONHOOK_INFO functionhook_info = {};
    //        functionhook_info.sName = sFunctionName;
    //        functionhook_info.nAddress = nFunctionAddress;

    //        g_mapFunctionHookInfos.insert(sFunctionName, functionhook_info);
    //    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::removeFunctionHook(const QString &sFunctionName)
{
    bool bResult = false;
    // TODO Check !!!

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    // TODO Check!
    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XInfoDB::BREAKPOINT breakPoint = m_listBreakpoints.at(i);

        if (breakPoint.vInfo.toString() == sFunctionName) {
            m_listBreakpoints.removeAt(i);
        }
    }

    if (m_mapFunctionHookInfos.contains(sFunctionName)) {
        m_mapFunctionHookInfos.remove(sFunctionName);

        bResult = true;
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
QMap<XADDR, XInfoDB::SHAREDOBJECT_INFO> *XInfoDB::getSharedObjectInfos()
{
    return &m_mapSharedObjectInfos;
}
#endif
#ifdef USE_XPROCESS
QList<XInfoDB::THREAD_INFO> *XInfoDB::getThreadInfos()
{
    return &m_listThreadInfos;
}
#endif
#ifdef USE_XPROCESS
QMap<QString, XInfoDB::FUNCTIONHOOK_INFO> *XInfoDB::getFunctionHookInfos()
{
    return &m_mapFunctionHookInfos;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::SHAREDOBJECT_INFO XInfoDB::findSharedInfoByName(const QString &sName)
{
    XInfoDB::SHAREDOBJECT_INFO result = {};

    for (QMap<XADDR, XInfoDB::SHAREDOBJECT_INFO>::iterator it = m_mapSharedObjectInfos.begin(); it != m_mapSharedObjectInfos.end();) {
        if (it.value().sName == sName) {
            result = it.value();

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
    XInfoDB::SHAREDOBJECT_INFO result = {};

    for (QMap<XADDR, XInfoDB::SHAREDOBJECT_INFO>::iterator it = m_mapSharedObjectInfos.begin(); it != m_mapSharedObjectInfos.end();) {
        XInfoDB::SHAREDOBJECT_INFO record = it.value();

        if ((record.nImageBase <= nAddress) && (record.nImageBase + record.nImageSize > nAddress)) {
            result = record;

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
    XInfoDB::THREAD_INFO result = {};

    m_pMutexThread->lock();

    qint32 nNumberOfRecords = m_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (m_listThreadInfos.at(i).nThreadID == nThreadID) {
            result = m_listThreadInfos.at(i);

            break;
        }
    }

    m_pMutexThread->unlock();

    return result;
}
#endif
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
XInfoDB::THREAD_INFO XInfoDB::findThreadInfoByHandle(X_HANDLE hThread)
{
    XInfoDB::THREAD_INFO result = {};

    m_pMutexThread->lock();

    qint32 nNumberOfRecords = m_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (m_listThreadInfos.at(i).hThread == hThread) {
            result = m_listThreadInfos.at(i);

            break;
        }
    }

    m_pMutexThread->unlock();

    return result;
}
#endif
#endif
#ifdef USE_XPROCESS
quint64 XInfoDB::getFunctionAddress(const QString &sFunctionName)
{
    Q_UNUSED(sFunctionName)
    // TODO
    return 0;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getAddressNextInstructionAfterCall(XADDR nAddress)
{
    Q_UNUSED(nAddress)

    XADDR nResult = -1;

    // QByteArray baData = read_array(nAddress, 15);

    // XCapstone::OPCODE_ID opcodeID = XCapstone::getOpcodeID(g_handle, nAddress, baData.data(), baData.size());

    // if (XCapstone::isCallOpcode(XBinary::getDisasmFamily(g_disasmMode), opcodeID.nOpcodeID)) {
    //     nResult = nAddress + opcodeID.nSize;
    // }

    return nResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::stepInto_Handle(X_HANDLE hThread, BPI bpInfo)
{
#ifndef Q_OS_WIN
    Q_UNUSED(hThread)
#endif
    XInfoDB::BREAKPOINT breakPoint = {};
    breakPoint.bpType = XInfoDB::BPT_CODE_STEP_FLAG;
    breakPoint.bpInfo = bpInfo;
#ifdef Q_OS_WIN
    breakPoint.nThreadID = findThreadInfoByHandle(hThread).nThreadID;
#endif
    return addBreakPoint(breakPoint);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::stepInto_Id(X_ID nThreadId, BPI bpInfo)
{
    XInfoDB::BREAKPOINT breakPoint = {};
    breakPoint.bpType = XInfoDB::BPT_CODE_STEP_FLAG;
    breakPoint.bpInfo = bpInfo;
    breakPoint.nThreadID = nThreadId;

    return addBreakPoint(breakPoint);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::_setStep_Handle(X_HANDLE hThread)
{
    bool bResult = false;

    if (hThread) {
#ifdef Q_OS_WIN
        CONTEXT context = {};
        context.ContextFlags = CONTEXT_CONTROL;  // EFLAGS

        if (GetThreadContext(hThread, &context)) {
            if (!(context.EFlags & 0x100)) {
                context.EFlags |= 0x100;
            }

            if (SetThreadContext(hThread, &context)) {
                bResult = true;
            }
        }
#endif
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::_setStep_Id(X_ID nThreadId)
{
    bool bResult = false;
#ifdef Q_OS_WIN
    X_HANDLE hThread = findThreadInfoByID(nThreadId).hThread;
    bResult = _setStep_Handle(hThread);
#endif
#ifdef Q_OS_LINUX
    user_regs_struct regs = {};
    errno = 0;

    if (ptrace(PTRACE_GETREGS, nThreadId, nullptr, &regs) != -1) {
        if (!(regs.eflags & 0x100)) {
            regs.eflags |= 0x100;
        }

        if (ptrace(PTRACE_SETREGS, nThreadId, nullptr, &regs) != -1) {
            bResult = true;
        }
    }
#endif
    // #ifdef Q_OS_LINUX
    //     errno = 0;

    //    long int nRet = ptrace(PTRACE_SINGLESTEP, nThreadId, 0, 0);

    //    if (nRet == 0) {
    //        bResult = true;
    //    } else {
    // #ifdef QT_DEBUG
    //        qDebug("ptrace failed: %s", strerror(errno));
    //        // TODO error signal
    // #endif
    //    }
    // #endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::suspendThread_Id(X_ID nThreadId)
{
    bool bResult = false;

#ifdef Q_OS_LINUX
    if (syscall(SYS_tgkill, m_processInfo.nProcessID, nThreadId, SIGSTOP) != -1) {
        // TODO Set thread status
        bResult = true;
    } else {
        qDebug("Cannot stop thread");
    }
#endif

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::suspendThread_Handle(X_HANDLE hThread)
{
    bool bResult = false;
#ifdef Q_OS_WIN
    bResult = (SuspendThread(hThread) != ((DWORD)-1));
#else
    Q_UNUSED(hThread)
#endif
#ifdef QT_DEBUG
//    qDebug("XInfoDB::suspendThread %X",hThread);
#endif

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::resumeThread_Id(X_ID nThreadId)
{
    bool bResult = false;

    if (getThreadStatus(nThreadId) == THREAD_STATUS_PAUSED) {
#ifdef Q_OS_LINUX
        if (ptrace(PTRACE_CONT, nThreadId, 0, 0) != -1) {
            bResult = setThreadStatus(nThreadId, THREAD_STATUS_RUNNING);
        }
#endif
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::resumeThread_Handle(X_HANDLE hThread)
{
    bool bResult = false;
#ifdef Q_OS_WIN
    bResult = (ResumeThread(hThread) != ((DWORD)-1));
#else
    Q_UNUSED(hThread)
#endif
#ifdef QT_DEBUG
//    qDebug("XInfoDB::resumeThread %X",hThread);
#endif

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::suspendAllThreads()
{
    bool bResult = false;

    m_pMutexThread->lock();

    qint32 nCount = m_listThreadInfos.count();

    // TODO Check if already suspended
    for (qint32 i = 0; i < nCount; i++) {
#ifdef Q_OS_WIN
        if (m_listThreadInfos.at(i).threadStatus == THREAD_STATUS_RUNNING) {
            if (suspendThread_Handle(m_listThreadInfos.at(i).hThread)) {
                m_listThreadInfos[i].threadStatus = THREAD_STATUS_PAUSED;
            }
        }
#endif
#ifdef Q_OS_LINUX
        if (syscall(SYS_tgkill, m_processInfo.nProcessID, m_listThreadInfos.at(i).nThreadID, SIGSTOP) != -1) {
            //            int thread_status=0;

            //            if(waitpid(pListThreads->at(i).nThreadID,&thread_status,__WALL)!=-1)
            //            {
            //                // TODO
            //            }
        } else {
            qDebug("Cannot stop thread");
        }
#endif
        bResult = true;
    }

    m_pMutexThread->unlock();

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::resumeAllThreads()
{
    bool bResult = false;

    m_pMutexThread->lock();

    qint32 nCount = m_listThreadInfos.count();

    // Resume all other threads
    for (qint32 i = 0; i < nCount; i++) {
#ifdef Q_OS_WIN
        if (m_listThreadInfos.at(i).threadStatus == THREAD_STATUS_PAUSED) {
            if (resumeThread_Handle(m_listThreadInfos.at(i).hThread)) {
                m_listThreadInfos[i].threadStatus = THREAD_STATUS_RUNNING;
            }
        }
#endif
#ifdef Q_OS_LINUX
        if (m_listThreadInfos.at(i).threadStatus == THREAD_STATUS_PAUSED) {
            if (ptrace(PTRACE_CONT, m_listThreadInfos.at(i).nThreadID, 0, 0) != -1) {
                m_listThreadInfos[i].threadStatus = THREAD_STATUS_RUNNING;
            }
        }
#endif

        bResult = true;
    }

    m_pMutexThread->unlock();

    return bResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::FUNCTION_INFO XInfoDB::getFunctionInfo(X_HANDLE hThread, const QString &sName)
{
    FUNCTION_INFO result = {};

#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_FULL;  // Full

    if (GetThreadContext(hThread, &context)) {
#ifdef Q_PROCESSOR_X86_32
        quint64 nSP = (quint32)(context.Esp);
        quint64 nIP = (quint32)(context.Eip);
#endif
#ifdef Q_PROCESSOR_X86_64
        quint64 nSP = (quint64)(context.Rsp);
        quint64 nIP = (quint64)(context.Rip);
#endif

        // TODO 64!
        result.nAddress = nIP;
        result.nRetAddress = read_uint32((quint32)nSP);

        for (qint32 i = 0; i < 10; i++)  // TODO consts
        {
            result.nParameters[i] = read_uint32((quint32)(nSP + 4 + i * 4));
        }

        result.sName = sName;
    }
#else
    Q_UNUSED(hThread)
    Q_UNUSED(sName)
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XHARDWAREBP XInfoDB::getHardwareBP_Handle(X_HANDLE hThread)
{
    // mb TODO Check Harware regs state and show MessageBox if program set DR
    XInfoDB::XHARDWAREBP result = {};

#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_DEBUG_REGISTERS;  // Full

    if (GetThreadContext(hThread, &context)) {
        quint64 nDR[8] = {};
#ifdef Q_PROCESSOR_X86_32
        nDR[0] = (quint32)(context.Dr0);
        nDR[1] = (quint32)(context.Dr1);
        nDR[2] = (quint32)(context.Dr2);
        nDR[3] = (quint32)(context.Dr3);
        nDR[6] = (quint32)(context.Dr6);
        nDR[7] = (quint32)(context.Dr7);
#endif
#ifdef Q_PROCESSOR_X86_64
        nDR[0] = (quint64)(context.Dr0);
        nDR[1] = (quint64)(context.Dr1);
        nDR[2] = (quint64)(context.Dr2);
        nDR[3] = (quint64)(context.Dr3);
        nDR[6] = (quint64)(context.Dr6);
        nDR[7] = (quint64)(context.Dr7);
#endif
        _regsToXHARDWAREBP(nDR, &result);
    }
#else
    Q_UNUSED(hThread)
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
// bool XInfoDB::setHardwareBP_Handle(X_HANDLE hThread, XInfoDB::XHARDWAREBP &hardwareBP)
//{
//     return false;
// }
#endif
#ifdef USE_XPROCESS
XInfoDB::XHARDWAREBP XInfoDB::getHardwareBP_Id(X_ID nThreadId)
{
    XInfoDB::XHARDWAREBP result = {};

#ifdef Q_OS_LINUX
#ifdef Q_PROCESSOR_X86_64
    quint64 nDR[8] = {};
    read_userData(nThreadId, offsetof(user, u_debugreg), (char *)(nDR), sizeof(nDR));

    _regsToXHARDWAREBP(nDR, &result);
#endif
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setHardwareBP_Id(X_ID nThreadId, XInfoDB::XHARDWAREBP &hardwareBP)
{
    Q_UNUSED(nThreadId)
    Q_UNUSED(hardwareBP)

    return false;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::_regsToXHARDWAREBP(quint64 *pDebugRegs, XInfoDB::XHARDWAREBP *pHardwareBP)
{
    // TODO check after step and CC bp
    bool bResult = true;

    quint64 nStatus = *(pDebugRegs + 6);
    quint64 nControl = *(pDebugRegs + 7);

    pHardwareBP->regs[0] =
        _bitsToXHARDWAREBP(*(pDebugRegs + 0), XBinary::getBitFromQword(nControl, 0), XBinary::getBitFromQword(nControl, 1), XBinary::getBitFromQword(nControl, 16),
                           XBinary::getBitFromQword(nControl, 17), XBinary::getBitFromQword(nControl, 18), XBinary::getBitFromQword(nControl, 19));
    pHardwareBP->regs[1] =
        _bitsToXHARDWAREBP(*(pDebugRegs + 1), XBinary::getBitFromQword(nControl, 2), XBinary::getBitFromQword(nControl, 3), XBinary::getBitFromQword(nControl, 20),
                           XBinary::getBitFromQword(nControl, 21), XBinary::getBitFromQword(nControl, 22), XBinary::getBitFromQword(nControl, 23));
    pHardwareBP->regs[2] =
        _bitsToXHARDWAREBP(*(pDebugRegs + 2), XBinary::getBitFromQword(nControl, 4), XBinary::getBitFromQword(nControl, 5), XBinary::getBitFromQword(nControl, 24),
                           XBinary::getBitFromQword(nControl, 25), XBinary::getBitFromQword(nControl, 26), XBinary::getBitFromQword(nControl, 27));
    pHardwareBP->regs[3] =
        _bitsToXHARDWAREBP(*(pDebugRegs + 3), XBinary::getBitFromQword(nControl, 6), XBinary::getBitFromQword(nControl, 7), XBinary::getBitFromQword(nControl, 28),
                           XBinary::getBitFromQword(nControl, 29), XBinary::getBitFromQword(nControl, 30), XBinary::getBitFromQword(nControl, 31));

    pHardwareBP->bSuccess[0] = XBinary::getBitFromQword(nStatus, 0);
    pHardwareBP->bSuccess[1] = XBinary::getBitFromQword(nStatus, 1);
    pHardwareBP->bSuccess[2] = XBinary::getBitFromQword(nStatus, 2);
    pHardwareBP->bSuccess[3] = XBinary::getBitFromQword(nStatus, 3);
    pHardwareBP->bSingleStep = XBinary::getBitFromQword(nStatus, 14);

    return bResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XHARDWAREBPREG XInfoDB::_bitsToXHARDWAREBP(quint64 nReg, bool bLocal, bool bGlobal, bool bCond0, bool bCond1, bool bSize0, bool bSize1)
{
    XInfoDB::XHARDWAREBPREG result = {};
    result.nAddress = nReg;
    result.bLocal = bLocal;
    result.bGlobal = bGlobal;

    if (bCond0 && bCond1) {
        result.bRead = true;
        result.bWrite = true;
    } else if (bCond0) {
        result.bWrite = true;
    } else {
        result.bExec = true;
    }

    if (bSize0 && bSize1) {
        result.nSize = 4;
    } else if (bSize0) {
        result.nSize = 2;
    } else if (bSize1) {
        result.nSize = 8;  // 64 only
    } else {
        result.nSize = 1;
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
// bool XInfoDB::_XHARDWAREBPToRegs(XInfoDB::XHARDWAREBP *pHardwareBP, quint64 *pDebugRegs)
//{
//     bool bResult = false;

//    return bResult;
//}
#endif
// #ifdef USE_XPROCESS
//  bool XInfoDB::setStep(XProcess::HANDLEID handleThread)
//{
//     bool bResult=true;
// #if defined(Q_OS_LINUX)
//     if(ptrace(PTRACE_SINGLESTEP,handleThread.nID,0,0))
//     {
//         bResult=true;
////        int wait_status;
////        waitpid(g_hidThread.nID,&wait_status,0);
//    }
// #endif

//    return bResult;
//}
// #endif
// #ifdef USE_XPROCESS
// bool XInfoDB::stepInto(XProcess::HANDLEID handleThread)
//{
//    XInfoDB::BREAKPOINT breakPoint={};
//    breakPoint.bpType=XInfoDB::BPT_CODE_HARDWARE;
//    breakPoint.bpInfo=XInfoDB::BPI_STEPINTO;

//    g_mapThreadBreakpoints.insert(handleThread.nID,breakPoint);

//    return setStep(handleThread);
//}
// #endif
// #ifdef USE_XPROCESS
// bool XInfoDB::resumeThread(XProcess::HANDLEID handleThread)
//{
//    bool bResult=false;

// #if defined(Q_OS_LINUX)
//     if(ptrace(PTRACE_CONT,handleThread.nID,0,0))
//     {
//         bResult=true;
////        int wait_status;
////        waitpid(handleThread.nID,&wait_status,0);
//    }
// #endif

//    return bResult;
//}
// #endif
#ifdef USE_XPROCESS
void XInfoDB::setProcessInfo(PROCESS_INFO processInfo)
{
    m_processInfo = processInfo;
    // m_mode = MODE_PROCESS;

    // g_nMainModuleAddress = processInfo.nImageBase;
    // g_nMainModuleSize = processInfo.nImageSize;
    // g_sMainModuleName = g_processInfo.sBaseFileName;
    // g_MainModuleMemoryMap=XFormats::getMemoryMap(XBinary::FT_REGION,0,true,)
    // // TODO getRegionMemoryMap
    // #ifdef USE_XPROCESS
    // #ifdef Q_PROCESSOR_X86_32
    //     g_disasmMode = XBinary::DM_X86_32;
    // #endif
    // #ifdef Q_PROCESSOR_X86_64
    //     g_disasmMode = XBinary::DM_X86_64;
    // #endif
    // #endif

    //     XCapstone::closeHandle(&g_handle);
    //     XCapstone::openHandle(g_disasmMode, &g_handle, true);

    _createTableNames();

    initDB();

    // initHexDb();
    // initDisasmDb();  // TODO Check
}
#endif
#ifdef USE_XPROCESS
XInfoDB::PROCESS_INFO *XInfoDB::getProcessInfo()
{
    return &m_processInfo;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateRegsById(X_ID nThreadId, const XREG_OPTIONS &regOptions)
{
    // TODO HASH !!!
    m_statusCurrent.listRegsPrev = m_statusCurrent.listRegs;  // TODO save nThreadID

    m_statusCurrent.listRegs.clear();
    m_statusCurrent.nThreadId = nThreadId;

#ifdef Q_OS_LINUX
    if (regOptions.bGeneral || regOptions.bIP || regOptions.bFlags || regOptions.bSegments) {
        user_regs_struct regs = {};
        //    user_regs_struct regs;
        errno = 0;

        if (ptrace(PTRACE_GETREGS, nThreadId, nullptr, &regs) != -1) {
            if (regOptions.bGeneral) {
                _addCurrentRegRecord(XREG_RAX, XBinary::getXVariant((quint64)(regs.rax)));
                _addCurrentRegRecord(XREG_RBX, XBinary::getXVariant((quint64)(regs.rbx)));
                _addCurrentRegRecord(XREG_RCX, XBinary::getXVariant((quint64)(regs.rcx)));
                _addCurrentRegRecord(XREG_RDX, XBinary::getXVariant((quint64)(regs.rdx)));
                _addCurrentRegRecord(XREG_RBP, XBinary::getXVariant((quint64)(regs.rbp)));
                _addCurrentRegRecord(XREG_RSP, XBinary::getXVariant((quint64)(regs.rsp)));
                _addCurrentRegRecord(XREG_RSI, XBinary::getXVariant((quint64)(regs.rsi)));
                _addCurrentRegRecord(XREG_RDI, XBinary::getXVariant((quint64)(regs.rdi)));
                _addCurrentRegRecord(XREG_R8, XBinary::getXVariant((quint64)(regs.r8)));
                _addCurrentRegRecord(XREG_R9, XBinary::getXVariant((quint64)(regs.r9)));
                _addCurrentRegRecord(XREG_R10, XBinary::getXVariant((quint64)(regs.r10)));
                _addCurrentRegRecord(XREG_R11, XBinary::getXVariant((quint64)(regs.r11)));
                _addCurrentRegRecord(XREG_R12, XBinary::getXVariant((quint64)(regs.r12)));
                _addCurrentRegRecord(XREG_R13, XBinary::getXVariant((quint64)(regs.r13)));
                _addCurrentRegRecord(XREG_R14, XBinary::getXVariant((quint64)(regs.r14)));
                _addCurrentRegRecord(XREG_R15, XBinary::getXVariant((quint64)(regs.r15)));
            }

            if (regOptions.bIP) {
                _addCurrentRegRecord(XREG_RIP, XBinary::getXVariant((quint64)(regs.rip)));
            }

            if (regOptions.bFlags) {
                _addCurrentRegRecord(XREG_RFLAGS, XBinary::getXVariant((quint64)(regs.eflags)));
            }

            if (regOptions.bSegments) {
                _addCurrentRegRecord(XREG_GS, XBinary::getXVariant((quint16)(regs.gs)));
                _addCurrentRegRecord(XREG_FS, XBinary::getXVariant((quint16)(regs.fs)));
                _addCurrentRegRecord(XREG_ES, XBinary::getXVariant((quint16)(regs.es)));
                _addCurrentRegRecord(XREG_DS, XBinary::getXVariant((quint16)(regs.ds)));
                _addCurrentRegRecord(XREG_CS, XBinary::getXVariant((quint16)(regs.cs)));
                _addCurrentRegRecord(XREG_SS, XBinary::getXVariant((quint16)(regs.ss)));
            }
        }
    } else {
#ifdef QT_DEBUG
        qDebug("errno: %s", strerror(errno));
#endif
    }

    if (regOptions.bDebug) {
#ifdef Q_PROCESSOR_X86_64
        quint64 debugRegs[8] = {};
        read_userData(nThreadId, offsetof(user, u_debugreg), (char *)(debugRegs), sizeof(debugRegs));
        _addCurrentRegRecord(XREG_DR0, XBinary::getXVariant((quint64)(debugRegs[0])));
        _addCurrentRegRecord(XREG_DR1, XBinary::getXVariant((quint64)(debugRegs[1])));
        _addCurrentRegRecord(XREG_DR2, XBinary::getXVariant((quint64)(debugRegs[2])));
        _addCurrentRegRecord(XREG_DR3, XBinary::getXVariant((quint64)(debugRegs[3])));
        _addCurrentRegRecord(XREG_DR6, XBinary::getXVariant((quint64)(debugRegs[6])));
        _addCurrentRegRecord(XREG_DR7, XBinary::getXVariant((quint64)(debugRegs[7])));
#endif
    }

    if (regOptions.bFloat || regOptions.bXMM || regOptions.bYMM) {
#ifdef Q_PROCESSOR_X86_64
        S_X86XState state = {};
        struct iovec io;
        io.iov_base = &state;
        io.iov_len = sizeof(state);

        if (ptrace(PTRACE_GETREGSET, nThreadId, (void *)NT_X86_XSTATE, (void *)&io) == -1) printf("BAD REGISTER REQUEST\n");
        // qDebug("io.iov_len %x", io.iov_len);
        //         qDebug("u_tsize %llX", _userData.u_tsize);
        //         qDebug("u_dsize %llX", _userData.u_dsize);
        //         qDebug("u_ssize %llX", _userData.u_ssize);
        //         qDebug("start_code %llX", _userData.start_code);
        //         qDebug("start_stack %llX", _userData.start_stack);

        if (regOptions.bFloat) {
            _addCurrentRegRecord(XREG_FPCR, XBinary::getXVariant((quint16)(state.cwd)));
            _addCurrentRegRecord(XREG_FPSR, XBinary::getXVariant((quint16)(state.swd)));
            _addCurrentRegRecord(XREG_FPTAG, XBinary::getXVariant((quint8)(state.twd)));
            //            _addCurrentRegRecord(XREG_FPIOFF, XBinary::getXVariant((quint32)(_userData.i387.fop))); // TODO Check
            //            _addCurrentRegRecord(XREG_FPISEL, XBinary::getXVariant((quint16)(context.FltSave.ErrorSelector))); // TODO Check
            //            _addCurrentRegRecord(XREG_FPDOFF, XBinary::getXVariant((quint32)(context.FltSave.DataOffset)));
            //            _addCurrentRegRecord(XREG_FPDSEL, XBinary::getXVariant((quint16)(context.FltSave.DataSelector)));
            for (qint32 i = 0; i < 8; i++) {
                _addCurrentRegRecord(XREG(XREG_ST0 + i), XBinary::getXVariant((quint64)(state.st_space[i].Low), (quint64)(state.st_space[i].High)));
            }
        }

        if (regOptions.bXMM) {
            _addCurrentRegRecord(XREG_MXCSR, XBinary::getXVariant((quint32)(state.mxcsr)));
            _addCurrentRegRecord(XREG_MXCSR_MASK, XBinary::getXVariant((quint32)(state.mxcsr_mask)));

            for (qint32 i = 0; i < 16; i++) {
                _addCurrentRegRecord(XREG(XREG_XMM0 + i), XBinary::getXVariant((quint64)(state.xmm_space[i].Low), (quint64)(state.xmm_space[i].High)));
            }
        }

        if (regOptions.bYMM) {
        }
#endif
    }

    emit registersListChanged();

//    __extension__ unsigned long long int orig_rax;
//    __extension__ unsigned long long int fs_base;
//    __extension__ unsigned long long int gs_base;
#endif
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateRegsByHandle(X_HANDLE hThread, const XREG_OPTIONS &regOptions)
{
    m_statusCurrent.hThread = hThread;

#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_ALL;  // All registers TODO Check regOptions
                                         // |CONTEXT_FLOATING_POINT|CONTEXT_EXTENDED_REGISTERS

    if (GetThreadContext(hThread, &context)) {
        quint32 nRegistersHash = XBinary::_getCRC32((char *)&context, sizeof(context), 0, XBinary::_getCRC32Table_EDB88320());

        if (m_statusCurrent.nRegistersHash != nRegistersHash) {
            m_statusCurrent.nRegistersHash = nRegistersHash;

            m_statusCurrent.listRegsPrev = m_statusCurrent.listRegs;  // TODO save nThreadID

            m_statusCurrent.listRegs.clear();

            if (regOptions.bGeneral) {
#ifdef Q_PROCESSOR_X86_32
                _addCurrentRegRecord(XREG_EAX, XBinary::getXVariant((quint32)(context.Eax)));
                _addCurrentRegRecord(XREG_EBX, XBinary::getXVariant((quint32)(context.Ebx)));
                _addCurrentRegRecord(XREG_ECX, XBinary::getXVariant((quint32)(context.Ecx)));
                _addCurrentRegRecord(XREG_EDX, XBinary::getXVariant((quint32)(context.Edx)));
                _addCurrentRegRecord(XREG_EBP, XBinary::getXVariant((quint32)(context.Ebp)));
                _addCurrentRegRecord(XREG_ESP, XBinary::getXVariant((quint32)(context.Esp)));
                _addCurrentRegRecord(XREG_ESI, XBinary::getXVariant((quint32)(context.Esi)));
                _addCurrentRegRecord(XREG_EDI, XBinary::getXVariant((quint32)(context.Edi)));
#endif
#ifdef Q_PROCESSOR_X86_64
                _addCurrentRegRecord(XREG_RAX, XBinary::getXVariant((quint64)(context.Rax)));
                _addCurrentRegRecord(XREG_RBX, XBinary::getXVariant((quint64)(context.Rbx)));
                _addCurrentRegRecord(XREG_RCX, XBinary::getXVariant((quint64)(context.Rcx)));
                _addCurrentRegRecord(XREG_RDX, XBinary::getXVariant((quint64)(context.Rdx)));
                _addCurrentRegRecord(XREG_RBP, XBinary::getXVariant((quint64)(context.Rbp)));
                _addCurrentRegRecord(XREG_RSP, XBinary::getXVariant((quint64)(context.Rsp)));
                _addCurrentRegRecord(XREG_RSI, XBinary::getXVariant((quint64)(context.Rsi)));
                _addCurrentRegRecord(XREG_RDI, XBinary::getXVariant((quint64)(context.Rdi)));
                _addCurrentRegRecord(XREG_R8, XBinary::getXVariant((quint64)(context.R8)));
                _addCurrentRegRecord(XREG_R9, XBinary::getXVariant((quint64)(context.R9)));
                _addCurrentRegRecord(XREG_R10, XBinary::getXVariant((quint64)(context.R10)));
                _addCurrentRegRecord(XREG_R11, XBinary::getXVariant((quint64)(context.R11)));
                _addCurrentRegRecord(XREG_R12, XBinary::getXVariant((quint64)(context.R12)));
                _addCurrentRegRecord(XREG_R13, XBinary::getXVariant((quint64)(context.R13)));
                _addCurrentRegRecord(XREG_R14, XBinary::getXVariant((quint64)(context.R14)));
                _addCurrentRegRecord(XREG_R15, XBinary::getXVariant((quint64)(context.R15)));
#endif
            }

            if (regOptions.bIP) {
#ifdef Q_PROCESSOR_X86_32
                _addCurrentRegRecord(XREG_EIP, XBinary::getXVariant((quint32)(context.Eip)));
#endif
#ifdef Q_PROCESSOR_X86_64
                _addCurrentRegRecord(XREG_RIP, XBinary::getXVariant((quint64)(context.Rip)));
#endif
            }

            if (regOptions.bFlags) {
#ifdef Q_PROCESSOR_X86_32
                _addCurrentRegRecord(XREG_EFLAGS, XBinary::getXVariant((quint32)(context.EFlags)));
#endif
#ifdef Q_PROCESSOR_X86_64
                _addCurrentRegRecord(XREG_RFLAGS,
                                     XBinary::getXVariant((quint64)(context.EFlags)));  // TODO !!!
#endif
            }

            if (regOptions.bSegments) {
                _addCurrentRegRecord(XREG_CS, XBinary::getXVariant((quint16)(context.SegCs)));
                _addCurrentRegRecord(XREG_FS, XBinary::getXVariant((quint16)(context.SegFs)));
                _addCurrentRegRecord(XREG_ES, XBinary::getXVariant((quint16)(context.SegEs)));
                _addCurrentRegRecord(XREG_DS, XBinary::getXVariant((quint16)(context.SegDs)));
                _addCurrentRegRecord(XREG_GS, XBinary::getXVariant((quint16)(context.SegGs)));
                _addCurrentRegRecord(XREG_SS, XBinary::getXVariant((quint16)(context.SegSs)));
            }

            if (regOptions.bDebug) {
#ifdef Q_PROCESSOR_X86_32
                _addCurrentRegRecord(XREG_DR0, XBinary::getXVariant((quint32)(context.Dr0)));
                _addCurrentRegRecord(XREG_DR1, XBinary::getXVariant((quint32)(context.Dr1)));
                _addCurrentRegRecord(XREG_DR2, XBinary::getXVariant((quint32)(context.Dr2)));
                _addCurrentRegRecord(XREG_DR3, XBinary::getXVariant((quint32)(context.Dr3)));
                _addCurrentRegRecord(XREG_DR6, XBinary::getXVariant((quint32)(context.Dr6)));
                _addCurrentRegRecord(XREG_DR7, XBinary::getXVariant((quint32)(context.Dr7)));
#endif
#ifdef Q_PROCESSOR_X86_64
                _addCurrentRegRecord(XREG_DR0, XBinary::getXVariant((quint64)(context.Dr0)));
                _addCurrentRegRecord(XREG_DR1, XBinary::getXVariant((quint64)(context.Dr1)));
                _addCurrentRegRecord(XREG_DR2, XBinary::getXVariant((quint64)(context.Dr2)));
                _addCurrentRegRecord(XREG_DR3, XBinary::getXVariant((quint64)(context.Dr3)));
                _addCurrentRegRecord(XREG_DR6, XBinary::getXVariant((quint64)(context.Dr6)));
                _addCurrentRegRecord(XREG_DR7, XBinary::getXVariant((quint64)(context.Dr7)));
#endif
            }

            if (regOptions.bFloat) {
#if defined(Q_PROCESSOR_X86_32)
//                for (qint32 i = 0; i < 8; i++) {
//                    _addCurrentRegRecord(
//                        XREG(XREG_ST0 + i), XBinary::getXVariant((quint64)(context.FloatSave.RegisterArea), (quint64)(context.FloatSave.FloatRegisters[i].High)));
//                }
#endif
#if defined(Q_PROCESSOR_X86_64)
                _addCurrentRegRecord(XREG_FPCR, XBinary::getXVariant((quint16)(context.FltSave.ControlWord)));
                _addCurrentRegRecord(XREG_FPSR, XBinary::getXVariant((quint16)(context.FltSave.StatusWord)));
                _addCurrentRegRecord(XREG_FPTAG, XBinary::getXVariant((quint8)(context.FltSave.TagWord)));
                //                _addCurrentRegRecord(XREG_FPIOFF, XBinary::getXVariant((quint32)(context.FltSave.ErrorOffset)));    // TODO Check
                //                _addCurrentRegRecord(XREG_FPISEL, XBinary::getXVariant((quint16)(context.FltSave.ErrorSelector)));  // TODO Check
                //                _addCurrentRegRecord(XREG_FPDOFF, XBinary::getXVariant((quint32)(context.FltSave.DataOffset)));
                //                _addCurrentRegRecord(XREG_FPDSEL, XBinary::getXVariant((quint16)(context.FltSave.DataSelector)));

                for (qint32 i = 0; i < 8; i++) {
                    _addCurrentRegRecord(XREG(XREG_ST0 + i),
                                         XBinary::getXVariant((quint64)(context.FltSave.FloatRegisters[i].Low), (quint64)(context.FltSave.FloatRegisters[i].High)));
                }
#endif
            }

            if (regOptions.bXMM) {
#if defined(Q_PROCESSOR_X86_32)
//                for (qint32 i = 0; i < 8; i++) {
//                    _addCurrentRegRecord(XREG(XREG_XMM0 + i),
//                                                   XBinary::getXVariant((quint64)(context.FltSave.XmmRegisters[i].Low),
//                                                   (quint64)(context.FltSave.XmmRegisters[i].High)));
//                }
#endif
#if defined(Q_PROCESSOR_X86_64)
                for (qint32 i = 0; i < 16; i++) {
                    _addCurrentRegRecord(XREG(XREG_XMM0 + i),
                                         XBinary::getXVariant((quint64)(context.FltSave.XmmRegisters[i].Low), (quint64)(context.FltSave.XmmRegisters[i].High)));
                }
                _addCurrentRegRecord(XREG_MXCSR, XBinary::getXVariant((quint32)(context.FltSave.MxCsr)));
                _addCurrentRegRecord(XREG_MXCSR_MASK, XBinary::getXVariant((quint32)(context.FltSave.MxCsr_Mask)));
#endif
            }

            if (regOptions.bYMM) {
#if defined(Q_PROCESSOR_X86_32)
                // TODO
#endif
#if defined(Q_PROCESSOR_X86_64)
                for (qint32 i = 0; i < 16; i++) {
                    _addCurrentRegRecord(XREG(XREG_YMM0 + i),
                                         XBinary::getXVariant((quint64)(context.FltSave.XmmRegisters[i].Low), (quint64)(context.FltSave.XmmRegisters[i].High)));
                }
#endif
                //            mapResult.insert("MxCsr",(quint32)(context.MxCsr));
            }

#ifdef QT_DEBUG
#if defined(Q_PROCESSOR_X86_64)
            //        qDebug("P1Home
            //        %s",XBinary::valueToHex((quint64)(context.P1Home)).toLatin1().data());
            //        qDebug("P2Home
            //        %s",XBinary::valueToHex((quint64)(context.P2Home)).toLatin1().data());
            //        qDebug("P3Home
            //        %s",XBinary::valueToHex((quint64)(context.P3Home)).toLatin1().data());
            //        qDebug("P4Home
            //        %s",XBinary::valueToHex((quint64)(context.P4Home)).toLatin1().data());
            //        qDebug("P5Home
            //        %s",XBinary::valueToHex((quint64)(context.P5Home)).toLatin1().data());
            //        qDebug("P6Home
            //        %s",XBinary::valueToHex((quint64)(context.P6Home)).toLatin1().data());
            //        qDebug("ContextFlags
            //        %s",XBinary::valueToHex((quint32)(context.ContextFlags)).toLatin1().data());
            //        qDebug("MxCsr
            //        %s",XBinary::valueToHex((quint32)(context.MxCsr)).toLatin1().data());

            //        qDebug("DebugControl
            //        %s",XBinary::valueToHex((quint64)(context.DebugControl)).toLatin1().data());
            //        qDebug("LastBranchToRip
            //        %s",XBinary::valueToHex((quint64)(context.LastBranchToRip)).toLatin1().data());
            //        qDebug("LastBranchFromRip
            //        %s",XBinary::valueToHex((quint64)(context.LastBranchFromRip)).toLatin1().data());
            //        qDebug("LastExceptionToRip
            //        %s",XBinary::valueToHex((quint64)(context.LastExceptionToRip)).toLatin1().data());
            //        qDebug("LastExceptionFromRip
            //        %s",XBinary::valueToHex((quint64)(context.LastExceptionFromRip)).toLatin1().data());
#endif
#endif
            emit registersListChanged();  // TODO mb hash
        }
    }
#else
    Q_UNUSED(regOptions)
#endif
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateMemoryRegionsList()
{
    // TODO mark new Regions
    // TODO watch changes
    //    g_statusPrev.listMemoryRegions = g_statusCurrent.listMemoryRegions;
#ifdef Q_OS_WIN
    quint32 nMemoryRegionsHash = XProcess::getMemoryRegionsListHash_Handle(m_processInfo.hProcess);
#endif
#ifdef Q_OS_LINUX
    quint32 nMemoryRegionsHash = XProcess::getMemoryRegionsListHash_Id(m_processInfo.nProcessID);
#endif
#ifdef Q_OS_MAC
    quint32 nMemoryRegionsHash = 0;  // TODO
#endif
    if (m_statusCurrent.nMemoryRegionsHash != nMemoryRegionsHash) {
        m_statusCurrent.nMemoryRegionsHash = nMemoryRegionsHash;
#ifdef Q_OS_WIN
        m_statusCurrent.listMemoryRegions = XProcess::getMemoryRegionsList_Handle(m_processInfo.hProcess, 0, 0xFFFFFFFFFFFFFFFF);
#endif
#ifdef Q_OS_LINUX
        m_statusCurrent.listMemoryRegions = XProcess::getMemoryRegionsList_Handle(m_processInfo.hProcessMemoryQuery, 0, 0xFFFFFFFFFFFFFFFF);
#endif
        emit memoryRegionsListChanged();
    }
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateModulesList()
{
    // mb TODO function for compare 2 lists
    //    g_statusPrev.listModules = g_statusCurrent.listModules;
    quint32 nModulesHash = XProcess::getModulesListHash(m_processInfo.nProcessID);

    if (m_statusCurrent.nModulesHash != nModulesHash) {
        m_statusCurrent.nModulesHash = nModulesHash;
        m_statusCurrent.listModules = XProcess::getModulesList(m_processInfo.nProcessID);

        emit modulesListChanged();
    }
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateThreadsList()
{
    // mb TODO function for compare 2 lists
    //    g_statusPrev.listModules = g_statusCurrent.listModules;
    quint32 nThreadsHash = XProcess::getThreadsListHash(m_processInfo.nProcessID);

    if (m_statusCurrent.nThreadsHash != nThreadsHash) {
        m_statusCurrent.nThreadsHash = nThreadsHash;
        m_statusCurrent.listThreads = XProcess::getThreadsList(m_processInfo.nProcessID);

        emit threadsListChanged();
    }
}
#endif
#ifdef USE_XPROCESS
XBinary::XVARIANT XInfoDB::getCurrentRegCache(XREG reg)
{
    return _getRegCache(&(m_statusCurrent.listRegs), reg);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentRegCache(XREG reg, XBinary::XVARIANT variant)
{
    _setRegCache(&(m_statusCurrent.listRegs), reg, variant);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentRegByThread(X_HANDLE hThread, XREG reg, XBinary::XVARIANT variant)
{
    bool bResult = false;
#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_ALL;  // All registers TODO Check regOptions
                                         // |CONTEXT_FLOATING_POINT|CONTEXT_EXTENDED_REGISTERS

    if (GetThreadContext(hThread, &context)) {
        bool bUnknownRegister = false;
#ifdef Q_PROCESSOR_X86
        // TODO flags
#ifdef Q_PROCESSOR_X86_32
        if (reg == XREG_EAX) context.Eax = variant.var.toULongLong();
        else if (reg == XREG_EBX) context.Ebx = variant.var.toULongLong();
        else if (reg == XREG_ECX) context.Ecx = variant.var.toULongLong();
        else if (reg == XREG_EDX) context.Edx = variant.var.toULongLong();
        else if (reg == XREG_EBP) context.Ebp = variant.var.toULongLong();
        else if (reg == XREG_ESP) context.Esp = variant.var.toULongLong();
        else if (reg == XREG_ESI) context.Esi = variant.var.toULongLong();
        else if (reg == XREG_EDI) context.Edi = variant.var.toULongLong();
        else if (reg == XREG_EFLAGS) context.EFlags = variant.var.toULongLong();
        else if (reg == XREG_EIP) context.Eip = variant.var.toULongLong();
        else if (reg == XREG_DR0) context.Dr0 = variant.var.toULongLong();
        else if (reg == XREG_DR1) context.Dr1 = variant.var.toULongLong();
        else if (reg == XREG_DR2) context.Dr2 = variant.var.toULongLong();
        else if (reg == XREG_DR3) context.Dr3 = variant.var.toULongLong();
        else if (reg == XREG_DR6) context.Dr6 = variant.var.toULongLong();
        else if (reg == XREG_DR7) context.Dr7 = variant.var.toULongLong();
        else if ((reg == XInfoDB::XREG_FLAGS_CF) || (reg == XInfoDB::XREG_FLAGS_PF) || (reg == XInfoDB::XREG_FLAGS_AF) || (reg == XInfoDB::XREG_FLAGS_ZF) ||
                 (reg == XInfoDB::XREG_FLAGS_SF) || (reg == XInfoDB::XREG_FLAGS_TF) || (reg == XInfoDB::XREG_FLAGS_IF) || (reg == XInfoDB::XREG_FLAGS_DF) ||
                 (reg == XInfoDB::XREG_FLAGS_OF)) {
            context.EFlags = setFlagToReg(XBinary::getXVariant((quint64)context.EFlags), reg, variant.var.v_bool).var.toULongLong();
        } else bUnknownRegister = true;
#endif
#ifdef Q_PROCESSOR_X86_64
        if (reg == XREG_RAX) context.Rax = variant.var.toULongLong();
        else if (reg == XREG_RBX) context.Rbx = variant.var.toULongLong();
        else if (reg == XREG_RCX) context.Rcx = variant.var.toULongLong();
        else if (reg == XREG_RDX) context.Rdx = variant.var.toULongLong();
        else if (reg == XREG_RBP) context.Rbp = variant.var.toULongLong();
        else if (reg == XREG_RSP) context.Rsp = variant.var.toULongLong();
        else if (reg == XREG_RSI) context.Rsi = variant.var.toULongLong();
        else if (reg == XREG_RDI) context.Rdi = variant.var.toULongLong();
        else if (reg == XREG_R8) context.R8 = variant.var.toULongLong();
        else if (reg == XREG_R9) context.R9 = variant.var.toULongLong();
        else if (reg == XREG_R10) context.R10 = variant.var.toULongLong();
        else if (reg == XREG_R11) context.R11 = variant.var.toULongLong();
        else if (reg == XREG_R12) context.R12 = variant.var.toULongLong();
        else if (reg == XREG_R13) context.R13 = variant.var.toULongLong();
        else if (reg == XREG_R14) context.R14 = variant.var.toULongLong();
        else if (reg == XREG_R15) context.R15 = variant.var.toULongLong();
        else if (reg == XREG_RFLAGS) context.EFlags = variant.var.toULongLong();
        else if (reg == XREG_RIP) context.Rip = variant.var.toULongLong();
        else if (reg == XREG_DR0) context.Dr0 = variant.var.toULongLong();
        else if (reg == XREG_DR1) context.Dr1 = variant.var.toULongLong();
        else if (reg == XREG_DR2) context.Dr2 = variant.var.toULongLong();
        else if (reg == XREG_DR3) context.Dr3 = variant.var.toULongLong();
        else if (reg == XREG_DR6) context.Dr6 = variant.var.toULongLong();
        else if (reg == XREG_DR7) context.Dr7 = variant.var.toULongLong();
        else if ((reg == XInfoDB::XREG_FLAGS_CF) || (reg == XInfoDB::XREG_FLAGS_PF) || (reg == XInfoDB::XREG_FLAGS_AF) || (reg == XInfoDB::XREG_FLAGS_ZF) ||
                 (reg == XInfoDB::XREG_FLAGS_SF) || (reg == XInfoDB::XREG_FLAGS_TF) || (reg == XInfoDB::XREG_FLAGS_IF) || (reg == XInfoDB::XREG_FLAGS_DF) ||
                 (reg == XInfoDB::XREG_FLAGS_OF)) {
            context.EFlags = setFlagToReg(XBinary::getXVariant((quint64)context.EFlags), reg, variant.var.v_bool).var.toULongLong();
        } else bUnknownRegister = true;
#endif
#endif
        // TODO more

        if (!bUnknownRegister) {
            if (SetThreadContext(hThread, &context)) {
                bResult = true;
            }
        } else {
#ifdef QT_DEBUG
            qDebug("Unknown register");
#endif
        }
    }
#else
    Q_UNUSED(hThread)
    Q_UNUSED(reg)
    Q_UNUSED(variant)
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentRegById(X_ID nThreadId, XREG reg, XBinary::XVARIANT variant)
{
    bool bResult = false;
#ifdef Q_OS_LINUX
    // TODO
    XREG_OPTIONS regOptions = getRegOptions(reg);

    if (regOptions.bGeneral || regOptions.bFlags || regOptions.bIP) {
        user_regs_struct regs = {};

        if (ptrace(PTRACE_GETREGS, nThreadId, nullptr, &regs) != -1) {
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
            if (reg == XREG_EAX) regs.eax = variant.var.toULongLong();
            else if (reg == XREG_EBX) regs.ebx = variant.var.toULongLong();
            else if (reg == XREG_ECX) regs.ecx = variant.var.toULongLong();
            else if (reg == XREG_EDX) regs.edx = variant.var.toULongLong();
            else if (reg == XREG_EBP) regs.ebp = variant.var.toULongLong();
            else if (reg == XREG_ESP) regs.esp = variant.var.toULongLong();
            else if (reg == XREG_ESI) regs.esi = variant.var.toULongLong();
            else if (reg == XREG_EDI) regs.edi = variant.var.toULongLong();
            else if (reg == XREG_EIP) regs.eip = variant.var.toULongLong();
            else if (reg == XREG_EFLAGS) regs.eflags = variant.var.toULongLong();
#endif
#ifdef Q_PROCESSOR_X86_64
            if (reg == XREG_RAX) regs.rax = variant.var.toULongLong();
            else if (reg == XREG_RBX) regs.rbx = variant.var.toULongLong();
            else if (reg == XREG_RCX) regs.rcx = variant.var.toULongLong();
            else if (reg == XREG_RDX) regs.rdx = variant.var.toULongLong();
            else if (reg == XREG_RBP) regs.rbp = variant.var.toULongLong();
            else if (reg == XREG_RSP) regs.rsp = variant.var.toULongLong();
            else if (reg == XREG_RSI) regs.rsi = variant.var.toULongLong();
            else if (reg == XREG_RDI) regs.rdi = variant.var.toULongLong();
            else if (reg == XREG_R8) regs.r8 = variant.var.toULongLong();
            else if (reg == XREG_R9) regs.r9 = variant.var.toULongLong();
            else if (reg == XREG_R10) regs.r10 = variant.var.toULongLong();
            else if (reg == XREG_R11) regs.r11 = variant.var.toULongLong();
            else if (reg == XREG_R12) regs.r12 = variant.var.toULongLong();
            else if (reg == XREG_R13) regs.r13 = variant.var.toULongLong();
            else if (reg == XREG_R14) regs.r14 = variant.var.toULongLong();
            else if (reg == XREG_R15) regs.r15 = variant.var.toULongLong();
            else if (reg == XREG_RIP) regs.rip = variant.var.toULongLong();
            else if (reg == XREG_RFLAGS) regs.eflags = variant.var.toULongLong();
#endif
#endif
            if (ptrace(PTRACE_SETREGS, nThreadId, nullptr, &regs) != -1) {
                bResult = true;
            }
        }
    }
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentReg(XREG reg, XBinary::XVARIANT variant)
{
    bool bResult = false;
#ifdef Q_OS_WIN
    bResult = setCurrentRegByThread(m_statusCurrent.hThread, reg, variant);
#endif
#ifdef Q_OS_LINUX
    bResult = setCurrentRegById(m_statusCurrent.nThreadId, reg, variant);
#endif
    return bResult;
}
#endif
#ifdef USE_XPROCESS
QList<XProcess::MEMORY_REGION> *XInfoDB::getCurrentMemoryRegionsList()
{
    return &(m_statusCurrent.listMemoryRegions);
}
#endif
#ifdef USE_XPROCESS
QList<XProcess::MODULE> *XInfoDB::getCurrentModulesList()
{
    return &(m_statusCurrent.listModules);
}
#endif
#ifdef USE_XPROCESS
QList<XProcess::THREAD_INFO> *XInfoDB::getCurrentThreadsList()
{
    return &(m_statusCurrent.listThreads);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::addBreakPoint(const BREAKPOINT &breakPoint)
{
    BREAKPOINT _breakPoint = breakPoint;

    bool bResult = false;

    if ((_breakPoint.bpType == BPT_CODE_SOFTWARE_DEFAULT) || (_breakPoint.bpType == BPT_UNKNOWN)) {
        _breakPoint.bpType = m_bpTypeDefault;
    }

    if (_breakPoint.sUUID == "") {
        _breakPoint.sUUID = XBinary::generateUUID();
    }

    if (!isBreakPointPresent(_breakPoint)) {
        if (_breakPoint.bpType == BPT_CODE_SOFTWARE_INT1) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\xF1", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_INT3) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\xCC", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_HLT) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\xF4", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_CLI) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\xFA", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_STI) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\xFB", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_INSB) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\x6C", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_INSD) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\x6D", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_OUTSB) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\x6E", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_OUTSD) {
            _breakPoint.nDataSize = 1;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\x6F", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_INT1LONG) {
            _breakPoint.nDataSize = 2;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\xCD\x01", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_INT3LONG) {
            _breakPoint.nDataSize = 2;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\xCD\x03", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_UD0) {
            _breakPoint.nDataSize = 2;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\x0F\xFF", _breakPoint.nDataSize);
        } else if (_breakPoint.bpType == BPT_CODE_SOFTWARE_UD2) {
            _breakPoint.nDataSize = 2;
            XBinary::_copyMemory(_breakPoint.bpData, (char *)"\x0F\x0B", _breakPoint.nDataSize);
        }

        m_listBreakpoints.append(_breakPoint);

        if (enableBreakPoint(_breakPoint.sUUID)) {
            bResult = true;
        } else {
            m_listBreakpoints.removeLast();
        }
    } else if ((_breakPoint.bpType == BPT_CODE_STEP_TO_RESTORE) || (_breakPoint.bpType == BPT_CODE_STEP_FLAG)) {
        bResult = _setStep_Id(_breakPoint.nThreadID);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::removeBreakPoint(const QString &sUUID)
{
    bool bResult = false;

    if (disableBreakPoint(sUUID)) {
        qint32 nNumberOfRecords = m_listBreakpoints.count();

        for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
            if (m_listBreakpoints.at(i).sUUID == sUUID) {
                m_listBreakpoints.removeAt(i);

                bResult = true;

                break;
            }
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::isBreakPointPresent(const BREAKPOINT &breakPoint)
{
    bool bResult = false;

    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if ((m_listBreakpoints.at(i).nAddress == breakPoint.nAddress) && (m_listBreakpoints.at(i).bpType == breakPoint.bpType) &&
            (m_listBreakpoints.at(i).nThreadID == breakPoint.nThreadID)) {
            bResult = true;
            break;
        }
    }
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::enableBreakPoint(const QString &sUUID)
{
    bool bResult = false;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (m_listBreakpoints.at(i).sUUID == sUUID) {
            if ((m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_HLT) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_CLI) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_STI) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSB) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSD) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSB) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSD) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1LONG) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3LONG) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD0) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD2)) {
                if (read_array(m_listBreakpoints.at(i).nAddress, m_listBreakpoints[i].origData, m_listBreakpoints.at(i).nDataSize) == m_listBreakpoints.at(i).nDataSize) {
                    if (write_array(m_listBreakpoints.at(i).nAddress, (char *)m_listBreakpoints.at(i).bpData, m_listBreakpoints.at(i).nDataSize)) {
                        bResult = true;
                    }
                }
            } else if ((m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR0) || (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR1) ||
                       (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR2) || (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR3)) {
                // TODO
            } else if ((m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_FLAG) || (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_TO_RESTORE)) {
                bResult = _setStep_Id(m_listBreakpoints.at(i).nThreadID);
            }

            break;
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::disableBreakPoint(const QString &sUUID)
{
    bool bResult = false;

    qint32 nNumberOfRecords = m_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (m_listBreakpoints.at(i).sUUID == sUUID) {
            if ((m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_HLT) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_CLI) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_STI) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSB) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSD) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSB) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSD) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1LONG) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3LONG) || (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD0) ||
                (m_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD2)) {
                if (write_array(m_listBreakpoints.at(i).nAddress, (char *)m_listBreakpoints.at(i).origData, m_listBreakpoints.at(i).nDataSize)) {
                    bResult = true;
                }
            } else if ((m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR0) || (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR1) ||
                       (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR2) || (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR3)) {
                // TODO
            } else if ((m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_FLAG) || (m_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_TO_RESTORE)) {
                // mb TODO
                bResult = true;
            }
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
QList<XInfoDB::BREAKPOINT> *XInfoDB::getBreakpoints()
{
    return &m_listBreakpoints;
}
#endif
#ifdef USE_XPROCESS
#ifdef Q_OS_LINUX
QMap<X_ID, XInfoDB::BREAKPOINT> *XInfoDB::getThreadBreakpoints()
{
    return &m_mapThreadBreakpoints;
}
#endif
#endif
#ifdef USE_XPROCESS
bool XInfoDB::isRegChanged(XREG reg)
{
    // mb TODO if init and 0 -> not changed
    XBinary::XVARIANT varReg = _getRegCache(&(g_statusCurrent.listRegs), reg);
    XBinary::XVARIANT varRegPrev = _getRegCache(&(g_statusCurrent.listRegsPrev), reg);

    return !(XBinary::isXVariantEqual(varReg, varRegPrev));
}
#endif
#ifdef USE_XPROCESS
QList<XInfoDB::REG_RECORD> XInfoDB::getCurrentRegs()
{
    return g_statusCurrent.listRegs;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentStackPointerCache()
{
    XADDR nResult = 0;

#ifdef Q_PROCESSOR_X86_32
    nResult = getCurrentRegCache(XInfoDB::XREG_ESP).var.toULongLong;
#endif
#ifdef Q_PROCESSOR_X86_64
    nResult = getCurrentRegCache(XInfoDB::XREG_RSP).var.toULongLong();
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentInstructionPointerCache()
{
    XADDR nResult = 0;

#ifdef Q_PROCESSOR_X86_32
    nResult = getCurrentRegCache(XInfoDB::XREG_EIP).var.toULongLong();
#endif
#ifdef Q_PROCESSOR_X86_64
    nResult = getCurrentRegCache(XInfoDB::XREG_RIP).var.toULongLong();
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentInstructionPointer_Handle(X_HANDLE hThread)
{
    XADDR nResult = 0;
#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_CONTROL;  // EIP

    if (GetThreadContext(hThread, &context)) {
#ifdef Q_PROCESSOR_X86_32
        nResult = context.Eip;
#endif
#ifdef Q_PROCESSOR_X86_64
        nResult = context.Rip;
#endif
    }
#else
    Q_UNUSED(hThread)
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentInstructionPointer_Id(X_ID nThreadId)
{
    XADDR nResult = 0;
#ifdef Q_OS_LINUX
    user_regs_struct regs = {};

    if (ptrace(PTRACE_GETREGS, nThreadId, nullptr, &regs) != -1) {
#if defined(Q_PROCESSOR_X86_64)
        nResult = regs.rip;
#elif defined(Q_PROCESSOR_X86_32)
        nResult = regs.eip;
#endif
    }
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentIntructionPointer_Handle(X_HANDLE hThread, XADDR nValue)
{
    bool bResult = false;
#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_CONTROL;  // EIP

    if (GetThreadContext(hThread, &context)) {
#ifdef Q_PROCESSOR_X86_32
        context.Eip = nValue;
#endif
#ifdef Q_PROCESSOR_X86_64
        context.Rip = nValue;
#endif
        if (SetThreadContext(hThread, &context)) {
            bResult = true;
        }
    }
#else
    Q_UNUSED(hThread)
    Q_UNUSED(nValue)
#endif
    return bResult;
}

bool XInfoDB::setCurrentIntructionPointer_Id(X_ID nThreadId, XADDR nValue)
{
    bool bResult = false;
#ifdef Q_OS_LINUX
    user_regs_struct regs = {};

    if (ptrace(PTRACE_GETREGS, nThreadId, nullptr, &regs) != -1) {
#if defined(Q_PROCESSOR_X86_64)
        regs.rip = nValue;
#elif defined(Q_PROCESSOR_X86_32)
        regs.eip = nValue;
#endif
        if (ptrace(PTRACE_SETREGS, nThreadId, nullptr, &regs) != -1) {
            bResult = true;
        }
    }
#endif
    return bResult;
}
#endif
// #ifdef USE_XPROCESS
// XCapstone::OPCODE_ID XInfoDB::getCurrentOpcode_Handle(X_HANDLE hThread)
// {
//     Q_UNUSED(hThread)
//     XCapstone::OPCODE_ID result = {};

//     // TODO

//     return result;
// }
// #endif
// #ifdef USE_XPROCESS
// XCapstone::OPCODE_ID XInfoDB::getCurrentOpcode_Id(X_ID nThreadId)
// {
//     Q_UNUSED(nThreadId)
//     XCapstone::OPCODE_ID result = {};

//     // TODO

//     return result;
// }
// #endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentStackPointer_Handle(X_HANDLE hThread)
{
    XADDR nResult = 0;
#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_CONTROL;

    if (GetThreadContext(hThread, &context)) {
#ifdef Q_PROCESSOR_X86_32
        nResult = (quint32)(context.Esp);
#endif
#ifdef Q_PROCESSOR_X86_64
        nResult = (quint64)(context.Rsp);
#endif
    }
#else
    Q_UNUSED(hThread)
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentStackPointer_Id(X_ID nThreadId)
{
    Q_UNUSED(nThreadId)
    XADDR nResult = 0;

    // TODO

    return nResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setCurrentStackPointer_Handle(X_HANDLE hThread, XADDR nValue)
{
    Q_UNUSED(hThread)
    Q_UNUSED(nValue)

    bool bResult = false;

    // TODO

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::isFunctionReturnAddress(XADDR nAddress)
{
    bool bResult = false;

    if (isAddressValid(nAddress)) {
        bResult = false;
    }
    // TODO

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::isAddressValid(XADDR nAddress)
{
    bool bResult = false;

    if (XProcess::getMemoryRegionByAddress(&g_statusCurrent.listMemoryRegions, nAddress).nSize != 0) {
        bResult = true;
    }

    return bResult;
}
#endif
// #ifdef USE_XPROCESS
//  void XInfoDB::_lockId(quint32 nId)
//{
//     QMutex *pMutex=nullptr;

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
// #endif
// #ifdef USE_XPROCESS
// void XInfoDB::_unlockID(quint32 nId)
//{
//    if(g_mapIds.contains(nId))
//    {
//        QMutex *pMutex=g_mapIds.value(nId);

//        pMutex->unlock();
//    }
//}
// #endif
// #ifdef USE_XPROCESS
// void XInfoDB::_waitID(quint32 nId)
//{
//    if(g_mapIds.contains(nId))
//    {
//        QMutex *pMutex=g_mapIds.value(nId);

//        pMutex->lock();
//        qDebug("TEST");
//        pMutex->unlock();
//    }
//}
// #endif

QList<XBinary::MEMORY_REPLACE> XInfoDB::getMemoryReplaces(quint64 nBase, quint64 nSize)
{
    QList<XBinary::MEMORY_REPLACE> listResult;
#ifdef USE_XPROCESS
    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XInfoDB::BREAKPOINT breakPoint = g_listBreakpoints.at(i);

        if (breakPoint.nDataSize) {
            if ((breakPoint.nAddress >= nBase) && (breakPoint.nAddress < nBase + nSize)) {
                XBinary::MEMORY_REPLACE record = {};
                record.nAddress = breakPoint.nAddress;
                record.baOriginal = QByteArray(breakPoint.origData, breakPoint.nDataSize);
                record.nSize = record.baOriginal.size();

                listResult.append(record);
            }
        }
    }
#else
    Q_UNUSED(nBase)
    Q_UNUSED(nSize)
#endif
    return listResult;
}

#ifdef USE_XPROCESS
QString XInfoDB::regIdToString(XREG reg)
{
    QString sResult = tr("Unknown");

    if (reg == XREG_NONE) sResult = QString("");
#ifdef Q_PROCESSOR_X86
    else if (reg == XREG_AX) sResult = QString("AX");
    else if (reg == XREG_CX) sResult = QString("CX");
    else if (reg == XREG_DX) sResult = QString("DX");
    else if (reg == XREG_BX) sResult = QString("BX");
    else if (reg == XREG_SP) sResult = QString("SP");
    else if (reg == XREG_BP) sResult = QString("BP");
    else if (reg == XREG_SI) sResult = QString("SI");
    else if (reg == XREG_DI) sResult = QString("DI");
    else if (reg == XREG_IP) sResult = QString("IP");
    else if (reg == XREG_FLAGS) sResult = QString("FLAGS");
    else if (reg == XREG_EAX) sResult = QString("EAX");
    else if (reg == XREG_ECX) sResult = QString("ECX");
    else if (reg == XREG_EDX) sResult = QString("EDX");
    else if (reg == XREG_EBX) sResult = QString("EBX");
    else if (reg == XREG_ESP) sResult = QString("ESP");
    else if (reg == XREG_EBP) sResult = QString("EBP");
    else if (reg == XREG_ESI) sResult = QString("ESI");
    else if (reg == XREG_EDI) sResult = QString("EDI");
    else if (reg == XREG_EIP) sResult = QString("EIP");
    else if (reg == XREG_EFLAGS) sResult = QString("EFLAGS");
#ifdef Q_PROCESSOR_X86_64
    else if (reg == XREG_RAX) sResult = QString("RAX");
    else if (reg == XREG_RCX) sResult = QString("RCX");
    else if (reg == XREG_RDX) sResult = QString("RDX");
    else if (reg == XREG_RBX) sResult = QString("RBX");
    else if (reg == XREG_RSP) sResult = QString("RSP");
    else if (reg == XREG_RBP) sResult = QString("RBP");
    else if (reg == XREG_RSI) sResult = QString("RSI");
    else if (reg == XREG_RDI) sResult = QString("RDI");
    else if (reg == XREG_R8) sResult = QString("R8");
    else if (reg == XREG_R9) sResult = QString("R9");
    else if (reg == XREG_R10) sResult = QString("R10");
    else if (reg == XREG_R11) sResult = QString("R11");
    else if (reg == XREG_R12) sResult = QString("R12");
    else if (reg == XREG_R13) sResult = QString("R13");
    else if (reg == XREG_R14) sResult = QString("R14");
    else if (reg == XREG_R15) sResult = QString("R15");
    else if (reg == XREG_RIP) sResult = QString("RIP");
    else if (reg == XREG_RFLAGS) sResult = QString("RFLAGS");
#endif
    else if (reg == XREG_CS) sResult = QString("CS");
    else if (reg == XREG_DS) sResult = QString("DS");
    else if (reg == XREG_ES) sResult = QString("ES");
    else if (reg == XREG_FS) sResult = QString("FS");
    else if (reg == XREG_GS) sResult = QString("GS");
    else if (reg == XREG_SS) sResult = QString("SS");
    else if (reg == XREG_DR0) sResult = QString("DR0");
    else if (reg == XREG_DR1) sResult = QString("DR1");
    else if (reg == XREG_DR2) sResult = QString("DR2");
    else if (reg == XREG_DR3) sResult = QString("DR3");
    else if (reg == XREG_DR6) sResult = QString("DR6");
    else if (reg == XREG_DR7) sResult = QString("DR7");
    else if (reg == XREG_FLAGS_CF) sResult = QString("CF");
    else if (reg == XREG_FLAGS_PF) sResult = QString("PF");
    else if (reg == XREG_FLAGS_AF) sResult = QString("AF");
    else if (reg == XREG_FLAGS_ZF) sResult = QString("ZF");
    else if (reg == XREG_FLAGS_SF) sResult = QString("SF");
    else if (reg == XREG_FLAGS_TF) sResult = QString("TF");
    else if (reg == XREG_FLAGS_IF) sResult = QString("IF");
    else if (reg == XREG_FLAGS_DF) sResult = QString("DF");
    else if (reg == XREG_FLAGS_OF) sResult = QString("OF");
    else if (reg == XREG_MXCSR) sResult = QString("MXCSR");
    else if (reg == XREG_MXCSR_MASK) sResult = QString("MXCSR_MASK");
    else if (reg == XREG_FPCR) sResult = QString("FPCR");
    else if (reg == XREG_FPSR) sResult = QString("FPSR");
    else if (reg == XREG_FPTAG) sResult = QString("FPTAG");
    //    else if (reg == XREG_FPIOFF) sResult = QString("FPIOFF");
    //    else if (reg == XREG_FPISEL) sResult = QString("FPISEL");
    //    else if (reg == XREG_FPDOFF) sResult = QString("FPDOFF");
    //    else if (reg == XREG_FPDSEL) sResult = QString("FPDSEL");
    else if ((reg >= XREG_ST0) && (reg <= XREG_ST7)) sResult = QString("ST%1").arg(reg - XREG_ST0);
    else if ((reg >= XREG_XMM0) && (reg <= XREG_XMM15)) sResult = QString("XM%1").arg(reg - XREG_XMM0);
    else if ((reg >= XREG_YMM0) && (reg <= XREG_YMM15)) sResult = QString("YM%1").arg(reg - XREG_YMM0);
    else if (reg == XREG_AH) sResult = QString("AH");
    else if (reg == XREG_CH) sResult = QString("CH");
    else if (reg == XREG_DH) sResult = QString("DH");
    else if (reg == XREG_BH) sResult = QString("BH");
    else if (reg == XREG_AL) sResult = QString("AL");
    else if (reg == XREG_CL) sResult = QString("CL");
    else if (reg == XREG_DL) sResult = QString("DL");
    else if (reg == XREG_BL) sResult = QString("BL");
#ifdef Q_PROCESSOR_X86_64
    else if (reg == XREG_SPL) sResult = QString("SPL");
    else if (reg == XREG_BPL) sResult = QString("BPL");
    else if (reg == XREG_SIL) sResult = QString("SIL");
    else if (reg == XREG_DIL) sResult = QString("DIL");
    else if (reg == XREG_R8D) sResult = QString("R8D");
    else if (reg == XREG_R9D) sResult = QString("R9D");
    else if (reg == XREG_R10D) sResult = QString("R10D");
    else if (reg == XREG_R11D) sResult = QString("R11D");
    else if (reg == XREG_R12D) sResult = QString("R12D");
    else if (reg == XREG_R13D) sResult = QString("R13D");
    else if (reg == XREG_R14D) sResult = QString("R14D");
    else if (reg == XREG_R15D) sResult = QString("R15D");
    else if (reg == XREG_R8W) sResult = QString("R8W");
    else if (reg == XREG_R9W) sResult = QString("R9W");
    else if (reg == XREG_R10W) sResult = QString("R10W");
    else if (reg == XREG_R11W) sResult = QString("R11W");
    else if (reg == XREG_R12W) sResult = QString("R12W");
    else if (reg == XREG_R13W) sResult = QString("R13W");
    else if (reg == XREG_R14W) sResult = QString("R14W");
    else if (reg == XREG_R15W) sResult = QString("R15W");
    else if (reg == XREG_R8B) sResult = QString("R8B");
    else if (reg == XREG_R9B) sResult = QString("R9B");
    else if (reg == XREG_R10B) sResult = QString("R10B");
    else if (reg == XREG_R11B) sResult = QString("R11B");
    else if (reg == XREG_R12B) sResult = QString("R12B");
    else if (reg == XREG_R13B) sResult = QString("R13B");
    else if (reg == XREG_R14B) sResult = QString("R14B");
    else if (reg == XREG_R15B) sResult = QString("R15B");
#endif
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg32(XREG reg)
{
    XREG result = XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_64
    if (reg == XREG_RAX) result = XREG_EAX;
    else if (reg == XREG_RCX) result = XREG_ECX;
    else if (reg == XREG_RDX) result = XREG_EDX;
    else if (reg == XREG_RBX) result = XREG_EBX;
    else if (reg == XREG_RSP) result = XREG_ESP;
    else if (reg == XREG_RBP) result = XREG_EBP;
    else if (reg == XREG_RSI) result = XREG_ESI;
    else if (reg == XREG_RDI) result = XREG_EDI;
    else if (reg == XREG_R8) result = XREG_R8D;
    else if (reg == XREG_R9) result = XREG_R9D;
    else if (reg == XREG_R10) result = XREG_R10D;
    else if (reg == XREG_R11) result = XREG_R11D;
    else if (reg == XREG_R12) result = XREG_R12D;
    else if (reg == XREG_R13) result = XREG_R13D;
    else if (reg == XREG_R14) result = XREG_R14D;
    else if (reg == XREG_R15) result = XREG_R15D;
    else if (reg == XREG_RIP) result = XREG_EIP;
    else if (reg == XREG_RFLAGS) result = XREG_EFLAGS;
#endif
#endif
    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg16(XREG reg)
{
    XREG result = XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
    if (reg == XREG_EAX) result = XREG_AX;
    else if (reg == XREG_ECX) result = XREG_CX;
    else if (reg == XREG_EDX) result = XREG_DX;
    else if (reg == XREG_EBX) result = XREG_BX;
    else if (reg == XREG_ESP) result = XREG_SP;
    else if (reg == XREG_EBP) result = XREG_BP;
    else if (reg == XREG_ESI) result = XREG_SI;
    else if (reg == XREG_EDI) result = XREG_DI;
    else if (reg == XREG_EIP) result = XREG_IP;
    else if (reg == XREG_EFLAGS) result = XREG_FLAGS;
#endif
#ifdef Q_PROCESSOR_X86_64
    if ((reg == XREG_RAX) || (reg == XREG_EAX)) result = XREG_AX;
    else if ((reg == XREG_RCX) || (reg == XREG_ECX)) result = XREG_CX;
    else if ((reg == XREG_RDX) || (reg == XREG_EDX)) result = XREG_DX;
    else if ((reg == XREG_RBX) || (reg == XREG_EBX)) result = XREG_BX;
    else if ((reg == XREG_RSP) || (reg == XREG_ESP)) result = XREG_SP;
    else if ((reg == XREG_RBP) || (reg == XREG_EBP)) result = XREG_BP;
    else if ((reg == XREG_RSI) || (reg == XREG_ESI)) result = XREG_SI;
    else if ((reg == XREG_RDI) || (reg == XREG_EDI)) result = XREG_DI;
    else if ((reg == XREG_R8) || (reg == XREG_R8D)) result = XREG_R8W;
    else if ((reg == XREG_R9) || (reg == XREG_R9D)) result = XREG_R9W;
    else if ((reg == XREG_R10) || (reg == XREG_R10D)) result = XREG_R10W;
    else if ((reg == XREG_R11) || (reg == XREG_R11D)) result = XREG_R11W;
    else if ((reg == XREG_R12) || (reg == XREG_R12D)) result = XREG_R12W;
    else if ((reg == XREG_R13) || (reg == XREG_R13D)) result = XREG_R13W;
    else if ((reg == XREG_R14) || (reg == XREG_R14D)) result = XREG_R14W;
    else if ((reg == XREG_R15) || (reg == XREG_R15D)) result = XREG_R15W;
    else if ((reg == XREG_RIP) || (reg == XREG_EIP)) result = XREG_IP;
    else if ((reg == XREG_RFLAGS) || (reg == XREG_EFLAGS)) result = XREG_FLAGS;
#endif
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg8H(XREG reg)
{
    XREG result = XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
    if ((reg == XREG_EAX) || (reg == XREG_AX)) result = XREG_AH;
    else if ((reg == XREG_ECX) || (reg == XREG_CX)) result = XREG_CH;
    else if ((reg == XREG_EDX) || (reg == XREG_DX)) result = XREG_DH;
    else if ((reg == XREG_EBX) || (reg == XREG_BX)) result = XREG_BH;
#endif
#ifdef Q_PROCESSOR_X86_64
    if ((reg == XREG_RAX) || (reg == XREG_EAX) || (reg == XREG_AX)) result = XREG_AH;
    else if ((reg == XREG_RCX) || (reg == XREG_ECX) || (reg == XREG_CX)) result = XREG_CH;
    else if ((reg == XREG_RDX) || (reg == XREG_EDX) || (reg == XREG_DX)) result = XREG_DH;
    else if ((reg == XREG_RBX) || (reg == XREG_EBX) || (reg == XREG_BX)) result = XREG_BH;
#endif
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG XInfoDB::getSubReg8L(XREG reg)
{
    XREG result = XREG_NONE;
#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
    if ((reg == XREG_EAX) || (reg == XREG_AX)) result = XREG_AL;
    else if ((reg == XREG_ECX) || (reg == XREG_CX)) result = XREG_CL;
    else if ((reg == XREG_EDX) || (reg == XREG_DX)) result = XREG_DL;
    else if ((reg == XREG_EBX) || (reg == XREG_BX)) result = XREG_BL;
#endif
#ifdef Q_PROCESSOR_X86_64
    if ((reg == XREG_RAX) || (reg == XREG_EAX) || (reg == XREG_AX)) result = XREG_AL;
    else if ((reg == XREG_RCX) || (reg == XREG_ECX) || (reg == XREG_CX)) result = XREG_CL;
    else if ((reg == XREG_RDX) || (reg == XREG_EDX) || (reg == XREG_DX)) result = XREG_DL;
    else if ((reg == XREG_RBX) || (reg == XREG_EBX) || (reg == XREG_BX)) result = XREG_BL;
    else if ((reg == XREG_RSP) || (reg == XREG_ESP) || (reg == XREG_SP)) result = XREG_SPL;
    else if ((reg == XREG_RBP) || (reg == XREG_EBP) || (reg == XREG_BP)) result = XREG_BPL;
    else if ((reg == XREG_RSI) || (reg == XREG_ESI) || (reg == XREG_SI)) result = XREG_SIL;
    else if ((reg == XREG_RDI) || (reg == XREG_EDI) || (reg == XREG_DI)) result = XREG_DIL;
    else if ((reg == XREG_R8) || (reg == XREG_R8D) || (reg == XREG_R8W)) result = XREG_R8B;
    else if ((reg == XREG_R9) || (reg == XREG_R9D) || (reg == XREG_R9W)) result = XREG_R9B;
    else if ((reg == XREG_R10) || (reg == XREG_R10D) || (reg == XREG_R10W)) result = XREG_R10B;
    else if ((reg == XREG_R11) || (reg == XREG_R11D) || (reg == XREG_R11W)) result = XREG_R11B;
    else if ((reg == XREG_R12) || (reg == XREG_R12D) || (reg == XREG_R12W)) result = XREG_R12B;
    else if ((reg == XREG_R13) || (reg == XREG_R13D) || (reg == XREG_R13W)) result = XREG_R13B;
    else if ((reg == XREG_R14) || (reg == XREG_R14D) || (reg == XREG_R14W)) result = XREG_R14B;
    else if ((reg == XREG_R15) || (reg == XREG_R15D) || (reg == XREG_R15W)) result = XREG_R15B;
#endif
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::XREG_OPTIONS XInfoDB::getRegOptions(XREG reg)
{
    XREG_OPTIONS result = {};

#ifdef Q_PROCESSOR_X86
#ifdef Q_PROCESSOR_X86_32
    if (reg == XREG_EAX) {
        result.bGeneral = true;
    }
#endif
#ifdef Q_PROCESSOR_X86_64
    if ((reg == XREG_RAX) || (reg == XREG_RBX) || (reg == XREG_RCX) || (reg == XREG_RDX) || (reg == XREG_RBP) || (reg == XREG_RSP) || (reg == XREG_RSI) ||
        (reg == XREG_RDI) || (reg == XREG_R8) || (reg == XREG_R9) || (reg == XREG_R10) || (reg == XREG_R11) || (reg == XREG_R12) || (reg == XREG_R13) ||
        (reg == XREG_R14) || (reg == XREG_R15)) {
        result.bGeneral = true;
    } else if (reg == XREG_RIP) {
        result.bIP = true;
    } else if (reg == XREG_RFLAGS) {
        result.bFlags = true;
    }
#endif
#endif

    return result;
}
#endif
#ifdef USE_XPROCESS
char *XInfoDB::allocateStringMemory(const QString &sFileName)
{
    char *pResult = nullptr;

    qint32 nSize = sFileName.length();

    pResult = new char[nSize + 1];
    XBinary::_zeroMemory(pResult, nSize + 1);
    XBinary::_copyMemory(pResult, sFileName.toUtf8().data(), nSize);

    return pResult;
}
#endif
XInfoDB::RECORD_INFO XInfoDB::getRecordInfo(quint64 nValue, RI_TYPE riType)
{
    Q_UNUSED(nValue)
    Q_UNUSED(riType)

    RECORD_INFO result = {};

    //     if ((nValue >= g_nMainModuleAddress) && (nValue < (g_nMainModuleAddress + g_nMainModuleSize))) {
    //         result.bValid = true;
    //         result.sModule = g_sMainModuleName;
    //         result.nAddress = nValue;
    //     }
    // #ifdef USE_XPROCESS
    //     else {
    //         // TODO mapRegions
    //         XProcess::MEMORY_REGION memoryRegion = XProcess::getMemoryRegionByAddress(&(g_statusCurrent.listMemoryRegions), nValue);

    //         if (memoryRegion.nSize) {
    //             result.bValid = true;
    //             result.nAddress = nValue;

    //             XProcess::MODULE _module = XProcess::getModuleByAddress(&(g_statusCurrent.listModules), nValue);

    //             if (_module.nSize) {
    //                 result.sModule = _module.sName;
    //             }
    //         }
    //     }
    // #endif

    //     if ((riType == RI_TYPE_GENERAL) || (riType == RI_TYPE_DATA) || (riType == RI_TYPE_ANSI) || (riType == RI_TYPE_UNICODE) || (riType == RI_TYPE_UTF8)) {
    //         if (result.bValid) {
    // #ifdef USE_XPROCESS
    //             result.baData = read_array(result.nAddress, 64);  // TODO const
    // #else
    //             qint64 nCurrentOffset = XBinary::addressToOffset(&g_MainModuleMemoryMap, result.nAddress);
    //             result.baData = read_array(nCurrentOffset, 64);  // TODO const
    // #endif
    //         }
    //     }

    //     if ((riType == RI_TYPE_GENERAL) || (riType == RI_TYPE_SYMBOL)) {
    //         if (result.bValid) {
    //             result.sSymbol = getSymbolStringByAddress(result.nAddress);

    //             if (riType == RI_TYPE_SYMBOL) {
    //                 if (result.sSymbol == "") {
    //                     result.sSymbol = QString("<%1.%2>").arg(result.sModule, XBinary::valueToHexOS(result.nAddress));
    //                 }
    //             }
    //         }
    //     }

    return result;
}

QString XInfoDB::recordInfoToString(RECORD_INFO recordInfo, RI_TYPE riType)
{
    QString sResult;

    if (recordInfo.bValid) {
        if (riType == RI_TYPE_GENERAL) {
            QString sAnsiString;
            QString sUnicodeString;

            quint8 nAnsiSymbol = XBinary::_read_uint8(recordInfo.baData.data());
            quint16 nUnicodeSymbol = XBinary::_read_uint16(recordInfo.baData.data());

            if ((nAnsiSymbol >= 8) && (nAnsiSymbol < 128)) {
                sAnsiString = QString::fromLatin1(recordInfo.baData);
            }

            if ((nUnicodeSymbol >= 8) && (nUnicodeSymbol < 128)) {
                sUnicodeString = QString::fromUtf16((quint16 *)(recordInfo.baData.data()), recordInfo.baData.size() / 2);  // TODO Check Qt6
            }

            qint32 nAnsiSize = sAnsiString.size();
            qint32 nUnicodeSize = sUnicodeString.size();

            if ((nAnsiSize >= nUnicodeSize) && (nAnsiSize > 5)) {
                sResult = QString("A: \"%1\"").arg(sAnsiString);
            } else if ((nUnicodeSize >= nAnsiSize) && (nUnicodeSize > 5)) {
                sResult = QString("U: \"%1\"").arg(sUnicodeString);
            } else if (recordInfo.sSymbol != "") {
                sResult = recordInfo.sSymbol;
            } else {
                sResult = QString("<%1.%2>").arg(recordInfo.sModule, XBinary::valueToHexOS(recordInfo.nAddress));
            }
        } else if (riType == RI_TYPE_ADDRESS) {
            sResult = QString("<%1.%2>").arg(recordInfo.sModule, XBinary::valueToHexOS(recordInfo.nAddress));
        } else if (riType == RI_TYPE_SYMBOL) {
            sResult = recordInfo.sSymbol;
        } else if (riType == RI_TYPE_ANSI) {
            sResult = QString::fromLatin1(recordInfo.baData);
        } else if (riType == RI_TYPE_UNICODE) {
            sResult = QString::fromUtf16((quint16 *)(recordInfo.baData.data()), recordInfo.baData.size() / 2);  // TODO Check Qt6
        } else if (riType == RI_TYPE_UTF8) {
            sResult = QString::fromUtf8(recordInfo.baData);
        } else if (riType == RI_TYPE_SYMBOLADDRESS) {
            sResult = recordInfoToString(recordInfo, RI_TYPE_SYMBOL);

            if (sResult == "") {
                sResult = recordInfoToString(recordInfo, RI_TYPE_ADDRESS);
            }
        }
    }

    return sResult;
}

bool XInfoDB::isSymbolPresent(XADDR nAddress)
{
    bool bResult = false;

    return bResult;
}

bool XInfoDB::isFunctionPresent(XADDR nAddress)
{
    bool bResult = false;

    return bResult;
}

#ifdef QT_SQL_LIB
bool XInfoDB::isTablePresent(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1'").arg(s_sql_tableName[dbTable]), false);

    return query.next();
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::isTableNotEmpty(QSqlDatabase *pDatabase, const QString &sTable)
{
    bool bResult = false;
    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("SELECT count(*) FROM '%1'").arg(sTable), false);

    if (query.next()) {
        bResult = (query.value(0).toInt() != 0);
    }

    return bResult;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::isTableNotEmpty(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    return isTableNotEmpty(pDatabase, s_sql_tableName[dbTable]);
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::isTablePresentAndNotEmpty(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    return (isTablePresent(pDatabase, dbTable) && isTableNotEmpty(pDatabase, dbTable));
}
#endif
#ifdef QT_SQL_LIB
void XInfoDB::createTable(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    QSqlQuery query(*pDatabase);

    if (dbTable == DBTABLE_BOOKMARKS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS BOOKMARKS ("
                         "UUID TEXT,"
                         "LOCATION INTEGER,"
                         "LOCTYPE INTEGER,"
                         "LOCSIZE INTEGER,"
                         "TEXTCOLOR TEXT,"
                         "BACKGROUNDCOLOR TEXT,"
                         "TEMPLATE TEXT,"
                         "COMMENT TEXT,"
                         "ISUSER INTEGER,"
                         "PRIMARY KEY (UUID)"
                         ")"),
                 false);
    } else if (dbTable == DBTABLE_SYMBOLS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS SYMBOLS ("
                         "FILETYPE INTEGER,"
                         "ADDRESS INTEGER,"
                         "SIZE INTEGER,"
                         "NAME TEXT,"
                         "FLAGS INTEGER,"
                         "BRANCH INTEGER,"
                         "PRIMARY KEY (ADDRESS, SIZE)"
                         ")"),
                 false);
    } else if (dbTable == DBTABLE_REFINFO) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS REFINFO ("
                         "FILETYPE INTEGER,"
                         "ADDRESS INTEGER,"
                         "REFADDRESS INTEGER,"
                         "SIZE INTEGER,"
                         "FLAGS INTEGER,"
                         "BRANCH INTEGER,"
                         "PRIMARY KEY (ADDRESS, SIZE)"
                         ")"),
                 false);
    } else if (dbTable == DBTABLE_RECORDS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS RECORDS ("
                         "FILETYPE INTEGER,"
                         "ADDRESS INTEGER,"
                         "SIZE INTEGER,"
                         "FLAGS INTEGER,"
                         "BRANCH INTEGER,"
                         "PRIMARY KEY (ADDRESS, SIZE)"
                         ")"),
                 false);
    }
}
#endif
#ifdef QT_SQL_LIB
void XInfoDB::removeTable(QSqlDatabase *pDatabase, const QString &sTable)
{
    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("DROP TABLE IF EXISTS %1").arg(sTable), false);
}
#endif
#ifdef QT_SQL_LIB
void XInfoDB::removeTable(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    removeTable(pDatabase, s_sql_tableName[dbTable]);
}
#endif
#ifdef QT_SQL_LIB
void XInfoDB::clearTable(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("DELETE FROM TABLE %1").arg(s_sql_tableName[dbTable]), false);
}
#endif
#ifdef QT_SQL_LIB
QString XInfoDB::getCreateSqlString(QSqlDatabase *pDatabase, const QString &sTable)
{
    QString sResult;

    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("SELECT sql FROM sqlite_master WHERE type='table' AND name='%1'").arg(sTable), false);

    if (query.next()) {
        sResult = query.value(0).toString();
    }

    return sResult;
}
#endif
#ifdef QT_SQL_LIB
QList<QString> XInfoDB::getNotEmptyTables(QSqlDatabase *pDatabase)
{
    QList<QString> listResult;

    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("SELECT name FROM sqlite_master WHERE type='table'"), false);

    while (query.next()) {
        QString sTable = query.value(0).toString();

        if (isTableNotEmpty(pDatabase, sTable)) {
            listResult.append(sTable);
        }
    }

    return listResult;
}
#endif
bool XInfoDB::isDbPresent()
{
    bool bResult = false;

    return bResult;
}

void XInfoDB::removeAllTables()
{
}

void XInfoDB::clearAllTables()
{
}
void XInfoDB::clearDb()
{
}

void XInfoDB::vacuumDb()
{
#ifdef QT_SQL_LIB
    // QSqlQuery query(g_dataBase);

    // querySQL(&query, QString("VACUUM"), false);
#endif
}

void XInfoDB::_addSymbolsFromFile(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct)
{
    m_pMutexSQL->lock();

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

    if (fileType == XBinary::FT_UNKNOWN) {
        fileType = XBinary::getPrefFileType(pDevice);
    }

    // #ifdef QT_SQL_LIB
    //     g_dataBase.transaction();
    // #endif

    if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) {
        XELF elf(pDevice, bIsImage, nModuleAddress);

        if (elf.isValid(pPdStruct)) {
            XBinary::_MEMORY_MAP memoryMap = elf.getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);

            if (memoryMap.nEntryPointAddress) {
                // _addSymbol(memoryMap.nEntryPointAddress, 0, "EntryPoint", SS_FILE);
                _addFunction(memoryMap.nEntryPointAddress, 0, "EntryPoint");
            }

            {
                QList<XELF_DEF::Elf_Phdr> listProgramHeaders = elf.getElf_PhdrList(1000);

                QList<XELF::TAG_STRUCT> listTagStructs = elf.getTagStructs(&listProgramHeaders, &memoryMap);

                QList<XELF::TAG_STRUCT> listDynSym = elf._getTagStructs(&listTagStructs, XELF_DEF::S_DT_SYMTAB);
                QList<XELF::TAG_STRUCT> listStrTab = elf._getTagStructs(&listTagStructs, XELF_DEF::S_DT_STRTAB);
                QList<XELF::TAG_STRUCT> listStrSize = elf._getTagStructs(&listTagStructs, XELF_DEF::S_DT_STRSZ);
                // TODO relocs !!!

                // TODO all sym Tables
                if (listDynSym.count() && listStrTab.count() && listStrSize.count()) {
                    qint64 nSymTabOffset = XBinary::addressToOffset(&memoryMap, listDynSym.at(0).nValue);
                    qint64 nSymTabSize = elf.getSymTableSize(nSymTabOffset);
                    qint64 nStringTableOffset = XBinary::addressToOffset(&memoryMap, listStrTab.at(0).nValue);
                    qint64 nStringTableSize = listStrSize.at(0).nValue;

                    _addELFSymbols(&elf, &memoryMap, nSymTabOffset, nSymTabSize, nStringTableOffset, nStringTableSize, pPdStruct);  // TODO delta
                }
            }
            {
                qint32 nSectionTableInex = elf.getSectionStringTable();
                QByteArray baStringTable = elf.getSection(nSectionTableInex);

                QList<XELF_DEF::Elf_Shdr> listSectionHeaders = elf.getElf_ShdrList(200);

                QList<XELF::SECTION_RECORD> listSectionRecords = elf.getSectionRecords(&listSectionHeaders, elf.isImage(), &baStringTable);

                QList<XELF::SECTION_RECORD> listSymTab = elf._getSectionRecords(&listSectionRecords, ".symtab");
                QList<XELF::SECTION_RECORD> listStrTab = elf._getSectionRecords(&listSectionRecords, ".strtab");

                if (listSymTab.count() && listStrTab.count()) {
                    qint64 nSymTabOffset = listSymTab.at(0).nOffset;
                    qint64 nSymTabSize = listSymTab.at(0).nSize;
                    qint64 nStringTableOffset = listStrTab.at(0).nOffset;
                    qint64 nStringTableSize = listStrTab.at(0).nSize;

                    _addELFSymbols(&elf, &memoryMap, nSymTabOffset, nSymTabSize, nStringTableOffset, nStringTableSize, pPdStruct);  // TODO delta
                }
            }
        }
    } else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) {
        XPE pe(pDevice, bIsImage, nModuleAddress);

        if (pe.isValid()) {
            QSet<XADDR> stAddresses;

            XBinary::_MEMORY_MAP memoryMap = pe.getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);

            // _addSymbol(memoryMap.nEntryPointAddress, 0, "EntryPoint", SS_FILE);  // TD mb tr
            _addFunction(memoryMap.nEntryPointAddress, 0, "EntryPoint");

            stAddresses.insert(memoryMap.nEntryPointAddress);

            // TODO PDB
            // TODO DWARF
            // TODO More
        }
    } else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) {
        XMACH mach(pDevice, bIsImage, nModuleAddress);

        if (mach.isValid()) {
            QSet<XADDR> stAddresses;

            XBinary::_MEMORY_MAP memoryMap = mach.getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);

            if (memoryMap.nEntryPointAddress != -1) {
                // _addSymbol(memoryMap.nEntryPointAddress, 0, "EntryPoint", SS_FILE);  // TD mb tr
                _addFunction(memoryMap.nEntryPointAddress, 0, "EntryPoint");

                stAddresses.insert(memoryMap.nEntryPointAddress);
            }
        }
    }

    // #ifdef QT_SQL_LIB
    //     g_dataBase.commit();
    //     vacuumDb();
    // #endif

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    m_pMutexSQL->unlock();
}

void XInfoDB::_addELFSymbols(XELF *pELF, XBinary::_MEMORY_MAP *pMemoryMap, qint64 nDataOffset, qint64 nDataSize, qint64 nStringsTableOffset, qint64 nStringsTableSize,
                             XBinary::PDSTRUCT *pPdStruct)
{
    QList<XELF_DEF::Elf_Sym> listSymbols = pELF->getElf_SymList(nDataOffset, nDataSize);
    qint32 nNumberOfSymbols = listSymbols.count();

    for (qint32 i = 0; (i < nNumberOfSymbols) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        XELF_DEF::Elf_Sym record = listSymbols.at(i);

        XADDR nSymbolAddress = record.st_value;
        quint64 nSymbolSize = record.st_size;

        // qint32 nBind = S_ELF64_ST_BIND(record.st_info);
        qint32 nType = S_ELF64_ST_TYPE(record.st_info);

        if (nSymbolAddress) {
            if ((nType == 0) || (nType == 1) || (nType == 2))  // NOTYPE,OBJECT,FUNC
                                                               // TODO consts
            {
                //                                XInfoDB::ST symbolType = XInfoDB::ST_LABEL;

                //                                if (nType == 0)
                //                                    symbolType = XInfoDB::ST_LABEL;
                //                                else if (nType == 1)
                //                                    symbolType = XInfoDB::ST_OBJECT;
                //                                else if (nType == 2)
                //                                    symbolType = XInfoDB::ST_FUNCTION;

                QString sSymbolName = pELF->getStringFromIndex(nStringsTableOffset, nStringsTableSize, record.st_name);

                if (XBinary::isAddressValid(pMemoryMap, nSymbolAddress)) {
                    if (!isSymbolPresent(nSymbolAddress)) {
                        //                                        _addSymbol(nSymbolAddress, nSymbolSize, 0, sSymbolName, symbolType, XInfoDB::SS_FILE);
                        // _addSymbol(nSymbolAddress, 0, sSymbolName, SS_FILE);
                        // TODO Add symbol ranges;
                    }

                    if (nType == 2) {
                        if (!isFunctionPresent(nSymbolAddress)) {
                            _addFunction(nSymbolAddress, nSymbolSize, sSymbolName);
                        }
                    }
                } else {
#ifdef QT_DEBUG
                    qDebug("%s", sSymbolName.toLatin1().data());
#endif
                }
            }
        }
    }
}

bool XInfoDB::_analyzeCode(const ANALYZEOPTIONS &analyzeOptions, XBinary::PDSTRUCT *pPdStruct)
{
#ifdef QT_DEBUG
    QElapsedTimer timer;
    timer.start();
#endif
    // mb TODO get all symbol info if init
    bool bResult = false;
#ifdef QT_SQL_LIB
    m_pMutexSQL->lock();

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

    // XBinary::DM disasmMode;
    //= getDisasmMode();
    // XBinary::DMFAMILY dmFamily = XBinary::getDisasmFamily(disasmMode);
    // XBinary::MODE mode = XBinary::getModeFromDisasmMode(disasmMode);

    XDisasmAbstract::DISASM_OPTIONS disasmOptions = {};

    XBinary binary(analyzeOptions.pDevice);
    qint64 nTotalSize = analyzeOptions.pDevice->size();

    QList<_ENTRY> listEntries;
    //    QList<XADDR> listSuspect;
    qint32 nBranch = _getBranchNumber();

    if (analyzeOptions.nStartAddress != (XADDR)-1) {
        _ENTRY _entry = {};
        _entry.nAddress = analyzeOptions.nStartAddress;
        _entry.nBranch = nBranch;
        listEntries.append(_entry);
        nBranch++;
    }

    XBinary::_MEMORY_RECORD mrCode = {};

    if (analyzeOptions.pMemoryMap->nEntryPointAddress) {
        // mb TODO for shared
        mrCode = XBinary::getMemoryRecordByAddress(analyzeOptions.pMemoryMap, analyzeOptions.pMemoryMap->nEntryPointAddress);
    }

    if (analyzeOptions.bAll) {
        //_addSymbols(analyzeOptions.pDevice, analyzeOptions.pMemoryMap->fileType, pPdStruct);

        QList<XADDR> listFunctionAddresses;

        listFunctionAddresses.append(getFunctionAddresses(pPdStruct));

        qint32 nNumberOfRecords = listFunctionAddresses.count();

        for (qint32 i = 0; XBinary::isPdStructNotCanceled(pPdStruct) && (i < nNumberOfRecords); i++) {
            _ENTRY _entry = {};
            _entry.nAddress = listFunctionAddresses.at(i);
            _entry.nBranch = nBranch;
            listEntries.append(_entry);
            nBranch++;
        }

        // Start of code section
        // mb optional
        //        if (mrCode.nSize) {
        //            listSuspect.append(mrCode.nAddress);
        //        }

        XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, analyzeOptions.pMemoryMap->nImageSize);

        // Import table
        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            // #ifdef QT_SQL_LIB
            //             g_dataBase.transaction();
            // #endif
            // QSqlQuery query(g_dataBase);

            // QList<XADDR> listImportAddresses = getImportSymbolAddresses(pPdStruct);
            // qint32 nNumberOfRecords = listImportAddresses.count();
            // qint32 nSize = 4;

            // for (qint32 i = 0; XBinary::isPdStructNotCanceled(pPdStruct)) && (i < nNumberOfRecords); i++) {
            //     XADDR nAddress = listImportAddresses.at(i);
            //     if (!_isShowRecordPresent(&query, nAddress, nSize)) {
            //         qint64 nOffset = XBinary::addressToOffset(analyzeOptions.pMemoryMap, nAddress);

            //         SHOWRECORD showRecord = {};
            //         showRecord.nAddress = nAddress;
            //         showRecord.nOffset = nOffset;
            //         showRecord.nSize = nSize;
            //         showRecord.recordType = RT_INTDATATYPE;
            //         showRecord.nRefTo = 0;
            //         showRecord.nRefFrom = 0;
            //         showRecord.nBranch = 0;
            //         showRecord.dbstatus = DBSTATUS_PROCESS;
            //         _addShowRecord(showRecord);
            //     }
            // }

            // #ifdef QT_SQL_LIB
            //             g_dataBase.commit();
            // #endif
        }
        // TODO Scan code section
        // XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, tr(""));
        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            vacuumDb();
        }
    }

    {
        // 5331
        // 4173
        qint32 N_BUFFERSIZE = 10000;

        qint64 nNumberOfOpcodes = 0;

        // QList<SHOWRECORD> listShowRecords;
        // QList<RELRECORD> listRelRecords;
        // QSet<XADDR> stShowRecords;

        // listShowRecords.reserve(N_BUFFERSIZE);
        // listRelRecords.reserve(N_BUFFERSIZE);
        // stShowRecords.reserve(N_BUFFERSIZE);

        // QSqlQuery queryRel(g_dataBase);
        // QSqlQuery queryShowRecords(g_dataBase);
        // QSqlQuery queryPresent(g_dataBase);

        // _addRelRecord_prepare(&queryRel);
        // _addShowRecord_prepare(&queryShowRecords);
        // _isShowRecordPresent_prepare1(&queryPresent);

        while (XBinary::isPdStructNotCanceled(pPdStruct)) {
            //        if ((!listEntries.isEmpty()) || (!listSuspect.isEmpty())) {
            if (!listEntries.isEmpty()) {
                XADDR nEntryAddress = 0;
                qint32 nCurrentBranch = 0;

                bool bSuspect = false;

                if (!listEntries.isEmpty()) {
                    nCurrentBranch = listEntries.first().nBranch;
                    nEntryAddress = listEntries.first().nAddress;
                } /*else if (!listSuspect.isEmpty()) {
                    nEntryAddress = listSuspect.first();
                    nBranch++;
                    nCurrentBranch = nBranch;
                    bSuspect = true;
                }*/

                XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, QString("%1: %2").arg(tr("Address"), XBinary::valueToHexEx(nEntryAddress)));

                // XADDR nCurrentAddress = nEntryAddress;

                // while XBinary::isPdStructNotCanceled(pPdStruct)) {
                //     if ((!stShowRecords.contains(nCurrentAddress)) && (!_isShowRecordPresent_bind1(&queryPresent, nCurrentAddress))) {
                //         qint64 nOffset = XBinary::addressToOffset(analyzeOptions.pMemoryMap, nCurrentAddress);

                //         if (nOffset != -1) {
                //             qint64 nSize = 16;

                //             nSize = qMin(nTotalSize - nOffset, nSize);

                //             nSize = binary.read_array(nOffset, byte_buffer, nSize);

                //             if (dmFamily == XBinary::DMFAMILY_X86) {
                //                 if (nSize >= 2) {
                //                     // mb TODO optional
                //                     // add byte ptr [rax], al
                //                     if ((*((quint16 *)byte_buffer) == 0) && (nCurrentAddress != analyzeOptions.nStartAddress)) {
                //                         break;
                //                     }
                //                 }
                //             }

                //             if (nSize > 0) {
                //                 XDisasmAbstract::DISASM_RESULT disasmResult = {};
                //                        // g_disasmCore.disAsm(byte_buffer, nSize, nCurrentAddress, disasmOptions);

                //                 if (disasmResult.bIsValid) {
                //                     {
                //                         quint64 _nCurrentBranch = nCurrentBranch;

                //                         if (bSuspect) {
                //                             if (XDisasmAbstract::isNopOpcode(dmFamily, disasmResult.nOpcode)) {
                //                                 _nCurrentBranch = 0;
                //                             } else if (XDisasmAbstract::isInt3Opcode(dmFamily, disasmResult.nOpcode)) {
                //                                 _nCurrentBranch = 0;
                //                             }
                //                         }

                //                         quint64 nRefTo = 0;

                //                         if (disasmResult.relType) {
                //                             nRefTo++;
                //                         }

                //                         if (disasmResult.memType) {
                //                             nRefTo++;
                //                         }

                //                         SHOWRECORD showRecord = {};
                //                         showRecord.bValid = true;
                //                         showRecord.nAddress = disasmResult.nAddress;
                //                         showRecord.nOffset = nOffset;
                //                         showRecord.nSize = disasmResult.nSize;
                //                         showRecord.recordType = RT_CODE;
                //                         showRecord.nRefTo = nRefTo;
                //                         showRecord.nRefFrom = 0;
                //                         showRecord.nBranch = _nCurrentBranch;
                //                         showRecord.dbstatus = DBSTATUS_PROCESS;
                //                         listShowRecords.append(showRecord);

                //                         if (disasmResult.relType || disasmResult.memType) {
                //                             RELRECORD relRecord = {};
                //                             relRecord.nAddress = disasmResult.nAddress;
                //                             relRecord.relType = disasmResult.relType;
                //                             relRecord.nXrefToRelative = disasmResult.nXrefToRelative;
                //                             relRecord.memType = disasmResult.memType;
                //                             relRecord.nXrefToMemory = disasmResult.nXrefToMemory;
                //                             relRecord.nMemorySize = disasmResult.nMemorySize;
                //                             relRecord.dbstatus = DBSTATUS_PROCESS;
                //                             listRelRecords.append(relRecord);
                //                         }

                //                         nNumberOfOpcodes++;

                //                         if (analyzeOptions.nCount && (nNumberOfOpcodes > analyzeOptions.nCount)) {
                //                             break;
                //                         }
                //                     }

                //                     stShowRecords.insert(nCurrentAddress);

                //                     nCurrentAddress += disasmResult.nSize;
                //                     XBinary::setPdStructCurrentIncrement(pPdStruct, _nFreeIndex);

                //                     if (disasmResult.relType) {
                //                         if (XDisasmAbstract::isCallOpcode(dmFamily, disasmResult.nOpcode)) {
                //                             nBranch++;
                //                             _ENTRY _entry = {};
                //                             _entry.nAddress = disasmResult.nXrefToRelative;
                //                             _entry.nBranch = nBranch;
                //                             listEntries.append(_entry);
                //                         } else {
                //                             _ENTRY _entry = {};
                //                             _entry.nAddress = disasmResult.nXrefToRelative;
                //                             _entry.nBranch = nCurrentBranch;
                //                             listEntries.append(_entry);
                //                         }

                //                         if (!(analyzeOptions.bAll)) {
                //                             // TODO relative if code code
                //                         }
                //                     }

                //                     if (stShowRecords.count() > N_BUFFERSIZE) {  // TODO Consts
                //                         _ENTRY _entry = {};
                //                         _entry.nAddress = nCurrentAddress;
                //                         _entry.nBranch = nCurrentBranch;
                //                         listEntries.append(_entry);

                //                         break;
                //                     }

                //                     // TODO Check mb int3
                //                     if (XDisasmAbstract::isRetOpcode(dmFamily, disasmResult.nOpcode)) {
                //                         //                                    if ((nCurrentAddress >= mrCode.nAddress) && (nCurrentAddress < (mrCode.nAddress +
                //                         //                                    mrCode.nSize))) {
                //                         //                                        listSuspect.append(nCurrentAddress);
                //                         //                                    }
                //                         break;
                //                     }

                //                     if (XDisasmAbstract::isInt3Opcode(dmFamily, disasmResult.nOpcode)) {
                //                         break;
                //                     }

                //                     if (XDisasmAbstract::isJumpOpcode(dmFamily, disasmResult.nOpcode)) {
                //                         break;
                //                     }
                //                     //                                if (dmFamily == XBinary::DMFAMILY_X86) {
                //                     //                                    // TODO Check
                //                     //                                    if ((disasmResult.sMnemonic == "ret") || (disasmResult.sMnemonic == "retn") ||
                //                     //                                    (disasmResult.sMnemonic == "jmp")) {
                //                     //                                        stEntries.insert(nCurrentAddress); // TODO optional
                //                     //                                        break;
                //                     //                                    }
                //                     //                                } else if ((dmFamily == XBinary::DMFAMILY_ARM) || (dmFamily == XBinary::DMFAMILY_ARM64)) {
                //                     //                                    // TODO Check
                //                     //                                    if ((disasmResult.sMnemonic == "b")) {
                //                     //                                        stEntries.insert(nCurrentAddress); // TODO optional
                //                     //                                        break;
                //                     //                                    }
                //                     //                                }
                //                 } else {
                //                     break;
                //                 }
                //             } else {
                //                 break;
                //             }
                //         } else {
                //             if (nCurrentAddress == nEntryAddress) {
                //                 // Virtual
                //                 SHOWRECORD showRecord = {};
                //                 showRecord.nAddress = nCurrentAddress;
                //                 showRecord.nOffset = -1;
                //                 showRecord.nSize = 1;
                //                 //                            showRecord.sRecText1 = "db";
                //                 //                            showRecord.sRecText2 = "0x01 dup (?)";
                //                 showRecord.recordType = RT_VIRTUAL;
                //                 showRecord.nRefTo = 0;
                //                 showRecord.nRefFrom = 0;
                //                 showRecord.dbstatus = DBSTATUS_PROCESS;
                //                 listShowRecords.append(showRecord);

                //                 nNumberOfOpcodes++;

                //                 if (analyzeOptions.nCount && (nNumberOfOpcodes > analyzeOptions.nCount)) {
                //                     break;
                //                 }
                //             }

                //             break;
                //         }
                //     } else {
                //         break;
                //     }
                // }

                if (!bSuspect) {
                    if (listEntries.first().nAddress == nEntryAddress) {
                        listEntries.removeFirst();
                    }
                } /*else {
                    if (listSuspect.first() == nEntryAddress) {
                        listSuspect.removeFirst();
                    }
                }*/

                // if (listEntries.isEmpty() || (listShowRecords.count() > N_BUFFERSIZE)) {
                //     // #ifdef QT_SQL_LIB
                //     //                     g_dataBase.transaction();
                //     // #endif
                //     // XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, tr("Save"));
                //     // _addShowRecords_bind(&queryShowRecords, &listShowRecords);
                //     // _addRelRecords_bind(&queryRel, &listRelRecords);
                //     // #ifdef QT_SQL_LIB
                //     //                     g_dataBase.commit();
                //     // #endif
                //     listShowRecords.clear();
                //     listRelRecords.clear();
                //     stShowRecords.clear();
                // }
            } else {
                break;
            }
        }

        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            vacuumDb();
        }
    }

    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        // #ifdef QT_SQL_LIB
        //         g_dataBase.transaction();
        // #endif
        QSet<XADDR> stCalls;

        // Functions

        {
            // Calls
            QList<XADDR> listLabels = getShowRecordRelAddresses(XDisasmAbstract::RELTYPE_CALL, DBSTATUS_PROCESS, pPdStruct);
            qint32 nNumberOfLabels = listLabels.count();

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
            XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, nNumberOfLabels);

            for (qint32 i = 0; XBinary::isPdStructNotCanceled(pPdStruct) && (i < nNumberOfLabels); i++) {
                XADDR nCallAddress = listLabels.at(i);
                stCalls.insert(nCallAddress);

                //            QString sSymbolName = QString("func_%1").arg(XBinary::valueToHexEx(nSymbolAddress));

                //            if (!isSymbolPresent(nSymbolAddress)) {
                //                _addSymbol(nSymbolAddress, 0, sSymbolName);
                //            }

                //            if (!isFunctionPresent(nSymbolAddress)) {
                //                _addFunction(nSymbolAddress, 0, sSymbolName);
                //            }

                XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
            }
        }
        {
            // Branches
            QList<XBinary::ADDRESSSIZE> listBranches = getBranches(DBSTATUS_PROCESS, pPdStruct);
            qint32 nNumberOfBranches = listBranches.count();

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
            XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, nNumberOfBranches);

            for (qint32 i = 0; XBinary::isPdStructNotCanceled(pPdStruct) && (i < nNumberOfBranches); i++) {
                XADDR nSymbolAddress = listBranches.at(i).nAddress;

                QString sSymbolName;

                if (stCalls.contains(nSymbolAddress)) {
                    sSymbolName = QString("sub_%1").arg(XBinary::valueToHexEx(nSymbolAddress));
                } else {
                    sSymbolName = QString("unk_%1").arg(XBinary::valueToHexEx(nSymbolAddress));
                }

                if (!isSymbolPresent(nSymbolAddress)) {
                    // _addSymbol(nSymbolAddress, 0, sSymbolName, SS_ANALYZE);
                }

                if (!isFunctionPresent(nSymbolAddress)) {
                    _addFunction(nSymbolAddress, listBranches.at(i).nSize, sSymbolName);
                } else {
                    updateFunctionSize(nSymbolAddress, listBranches.at(i).nSize);
                }

                XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
            }
        }

        // #ifdef QT_SQL_LIB
        //         g_dataBase.commit();
        // #endif
    }

    // Labels
    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        // #ifdef QT_SQL_LIB
        //         g_dataBase.transaction();
        // #endif
        QList<XADDR> listLabels = getShowRecordRelAddresses(XDisasmAbstract::RELTYPE_ALL, DBSTATUS_PROCESS, pPdStruct);
        qint32 nNumberOfLabels = listLabels.count();

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
        XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, nNumberOfLabels);

        for (qint32 i = 0; XBinary::isPdStructNotCanceled(pPdStruct) && (i < nNumberOfLabels); i++) {
            XADDR nSymbolAddress = listLabels.at(i);

            if (!isSymbolPresent(nSymbolAddress)) {
                QString sSymbolName = QString("label_%1").arg(XBinary::valueToHexEx(nSymbolAddress));

                // _addSymbol(nSymbolAddress, 0, sSymbolName, SS_ANALYZE);
            }

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
        }

        // #ifdef QT_SQL_LIB
        //         g_dataBase.commit();
        // #endif
    }

    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        vacuumDb();
    }

    // TODO Get all branches
    // TODO Get all functions and check size

    // Variables
    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        // #ifdef QT_SQL_LIB
        //         g_dataBase.transaction();
        // #endif
        //         QSqlQuery query(g_dataBase);

        //         QList<XBinary::ADDRESSSIZE> listVariables = getShowRecordMemoryVariables(DBSTATUS_PROCESS, pPdStruct);
        //         qint32 nNumberOfVariables = listVariables.count();

        //         for (qint32 i = 0; XBinary::isPdStructNotCanceled(pPdStruct)) && (i < nNumberOfVariables); i++) {
        //             XBinary::ADDRESSSIZE record = listVariables.at(i);

        //             //                // TODO if size = 0 check if it is a string
        //             //                QString sVarName = "db";
        //             //                if (record.nSize == 0) {
        //             //                    record.nSize = 1;
        //             //                } else if (record.nSize == 2) {
        //             //                    sVarName = "word";
        //             //                } else if (record.nSize == 4) {
        //             //                    sVarName = "dword";
        //             //                } else if (record.nSize == 8) {
        //             //                    sVarName = "qword";
        //             //                }

        //             bool bAdd = false;

        //             if (record.nSize) {
        //                 if (!_isShowRecordPresent(&query, record.nAddress, record.nSize)) {
        //                     bAdd = true;
        //                 }
        //             }

        //             if (bAdd) {
        //                 if (XBinary::isAddressValid(analyzeOptions.pMemoryMap, record.nAddress)) {
        //                     qint64 nOffset = XBinary::addressToOffset(analyzeOptions.pMemoryMap, record.nAddress);

        //                     SHOWRECORD showRecord = {};
        //                     showRecord.nAddress = record.nAddress;
        //                     showRecord.nOffset = nOffset;
        //                     showRecord.nSize = record.nSize;
        //                     showRecord.recordType = RT_INTDATATYPE;
        //                     showRecord.nRefTo = 0;
        //                     showRecord.nRefFrom = 0;
        //                     showRecord.nBranch = 0;
        //                     showRecord.dbstatus = DBSTATUS_PROCESS;
        //                     _addShowRecord(showRecord);
        //                 }
        //             }

        //             if (!isSymbolPresent(record.nAddress)) {
        //                 if (XBinary::isAddressValid(analyzeOptions.pMemoryMap, record.nAddress)) {
        //                     QString sSymbolName;

        //                     // TODO Check string
        //                     if (record.nSize) {
        //                         sSymbolName = QString("var_%1").arg(XBinary::valueToHexEx(record.nAddress));
        //                     } else {
        //                         sSymbolName = QString("label_%1").arg(XBinary::valueToHexEx(record.nAddress));
        //                     }

        //                     if (!_addSymbol(record.nAddress, 0, sSymbolName, SS_ANALYZE)) {
        // #ifdef QT_DEBUG
        //                         qDebug("%s", XBinary::valueToHex(record.nAddress).toLatin1().data());
        // #endif
        //                     }
        //                     // TODO ST_DATA_ANSISTRING
        //                 }
        //             }
        //         }

        // #ifdef QT_SQL_LIB
        //         g_dataBase.commit();
        // #endif
        // XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, tr(""));
        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            vacuumDb();
        }
    }

    // Update references
    if (analyzeOptions.bAll) {
        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            // #ifdef QT_SQL_LIB
            //             g_dataBase.transaction();
            // #endif
            // QList<RELRECORD> listRelRecords = getRelRecords(DBSTATUS_PROCESS);  // TODO optimize
            // // TODO reset counters!!!

            // qint32 nNumberOfRecords = listRelRecords.count();

            // for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct)); i++) {
            //     RELRECORD record = listRelRecords.at(i);

            //     if (record.relType) {
            //         _incShowRecordRefFrom(record.nXrefToRelative);
            //     }

            //     if (record.memType) {
            //         _incShowRecordRefFrom(record.nXrefToMemory);
            //     }
            // }
            // TODO

#ifdef QT_SQL_LIB
            // g_dataBase.commit();
            if (XBinary::isPdStructNotCanceled(pPdStruct)) {
                vacuumDb();
            }
#endif
        }
    }

    //    if (bIsInit) {
    //        if XBinary::isPdStructNotCanceled(pPdStruct)) {
    // #ifdef QT_SQL_LIB
    //            g_dataBase.transaction();
    // #endif

    //            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
    //            XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, pMemoryMap->nImageSize);

    //            // updateShowRecordLine
    //            quint64 nLineNumber = 0;

    //            for (XADDR nCurrentAddress = pMemoryMap->nModuleAddress;
    //                 XBinary::isPdStructNotCanceled(pPdStruct)) && (nCurrentAddress < (pMemoryMap->nModuleAddress + pMemoryMap->nImageSize));) {
    //                XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, QString("%1: %2").arg(tr("Address"), XBinary::valueToHexEx(nCurrentAddress)));

    //                SHOWRECORD showRecord = getShowRecordByAddress(nCurrentAddress);

    //                qint64 nRecordSize = showRecord.nSize;

    //                if (nRecordSize) {
    //                    updateShowRecordLine(nCurrentAddress, nLineNumber++);
    //                } else {
    //                    // TODO LoadSections limits
    //                    SHOWRECORD showRecordNext = getNextShowRecordByAddress(nCurrentAddress);

    //                    if (showRecordNext.nSize) {
    //                        nRecordSize = showRecordNext.nAddress - nCurrentAddress;
    //                    } else {
    //                        // TODO end of Image
    //                        nRecordSize = pMemoryMap->nModuleAddress + pMemoryMap->nImageSize - nCurrentAddress;
    //                    }

    //                    for (XADDR _nCurrentAddress = nCurrentAddress; XBinary::isPdStructNotCanceled(pPdStruct)) && (_nCurrentAddress < nCurrentAddress +
    //                    nRecordSize);) {
    //                        XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(pMemoryMap, _nCurrentAddress);

    //                        if (mr.nSize == 0) {
    //                            break;
    //                        }

    //                        qint64 _nRecordSize = qMin((qint64)((mr.nAddress + mr.nSize) - _nCurrentAddress), (qint64)((nCurrentAddress + nRecordSize) -
    //                        _nCurrentAddress)); qint64 _nOffset = XBinary::addressToOffset(pMemoryMap, _nCurrentAddress);

    //                        if (_nOffset == -1) {
    //                            QString sDataName = QString("0x%1 dup (?)").arg(QString::number(_nRecordSize, 16));
    //                            _addShowRecord(_nCurrentAddress, _nOffset, _nRecordSize, "db", sDataName, RT_DATA, nLineNumber++, 0, 0);
    //                        } else {
    //                            for (XADDR _nCurrentAddressData = _nCurrentAddress; XBinary::isPdStructNotCanceled(pPdStruct)) && (_nCurrentAddressData <
    //                            _nCurrentAddress + _nRecordSize);) {
    //                                qint64 _nOffsetData = XBinary::addressToOffset(pMemoryMap, _nCurrentAddressData);

    //                                qint64 _nRecordSizeData = 0;
    //                                QString sOpcodeName = "db";
    //                                QString sDataName;

    //                                //                            XBinary::REGION_FILL regionFill = {};

    //                                //                            regionFill = binary.getRegionFill(_nOffsetData, 16, 16);

    //                                //                            if (regionFill.nSize) {
    //                                //                                _nRecordSizeData = regionFill.nSize;
    //                                //                                sDataName = QString("0x%1 dup (0x%2)").arg(QString::number(_nRecordSizeData, 16),
    //                                //                                QString::number(regionFill.nByte, 16));
    //                                //                            } else {
    //                                //                                _nRecordSizeData = qMin((qint64)16, (qint64)((_nCurrentAddress + _nRecordSize) -
    //                                _nCurrentAddressData));
    //                                //                                // TODO consts
    //                                //                            }
    //                                _nRecordSizeData = qMin((qint64)16, (qint64)((_nCurrentAddress + _nRecordSize) - _nCurrentAddressData));  // TODO consts

    //                                _addShowRecord(_nCurrentAddressData, _nOffsetData, _nRecordSizeData, sOpcodeName, sDataName, RT_DATA, nLineNumber++, 0, 0);

    //                                _nCurrentAddressData += _nRecordSizeData;

    //                                XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nCurrentAddressData - pMemoryMap->nModuleAddress);
    //                            }
    //                        }

    //                        _nCurrentAddress += _nRecordSize;

    //                        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nCurrentAddress - pMemoryMap->nModuleAddress);
    //                    }
    //                }

    //                if (nRecordSize == 0) {
    //                    break;
    //                }

    //                nCurrentAddress += nRecordSize;

    //                XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nCurrentAddress - pMemoryMap->nModuleAddress);
    //            }

    // #ifdef QT_SQL_LIB
    //             g_dataBase.commit();
    //             // XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, tr(""));
    //             if XBinary::isPdStructNotCanceled(pPdStruct)) {
    //                 vacuumDb();
    //             }
    // #endif
    //         }
    //     }

    // mb TODO Overlay

    _completeDbAnalyze();

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    m_pMutexSQL->unlock();

    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        bResult = true;
    }
#else
    Q_UNUSED(analyzeOptions)
    Q_UNUSED(pPdStruct)
#endif

#ifdef QT_DEBUG
    qDebug("Analyze all %lld", timer.elapsed());
#endif

    return bResult;
}

bool XInfoDB::_analyze(XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct)
{
    if (!m_mapProfiles.contains(fileType)) {
        return false;
    }

    STATE *pState = getState(fileType);

    pState->nCurrentBranch = 0;

    pState->listSymbols.clear();
    pState->listRecords.clear();
    pState->listRefs.clear();
    pState->listStrings.clear();

    if ((fileType == XBinary::FT_MACHO32) || (fileType == XBinary::FT_MACHO64)) {
        XMACH mach(pState->pDevice, pState->bIsImage, pState->nModuleAddress);

        if (mach.isValid()) {
            pState->memoryMap = mach.getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);
            bool bIs64 = (pState->memoryMap.mode == XBinary::MODE_64);

            QList<XMACH::COMMAND_RECORD> listCR = mach.getCommandRecords(0, pPdStruct);

            qint64 nOffsetSymTab = mach.getCommandRecordOffset(XMACH_DEF::S_LC_SYMTAB, 0, &listCR);

            if (nOffsetSymTab != -1) {
                XMACH_DEF::symtab_command symtab = mach._read_symtab_command(nOffsetSymTab);

                QByteArray baStringTable = mach.read_array_process(symtab.stroff, symtab.strsize, pPdStruct);
                char *pBuffer = baStringTable.data();
                qint64 nBufferSize = baStringTable.size();

                nOffsetSymTab = symtab.symoff;

                for (qint32 i = 0; (i < (int)(symtab.nsyms)) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                    qint32 nIndex = 0;
                    XADDR nAddress = 0;

                    if (bIs64) {
                        XMACH_DEF::nlist_64 _nlist = mach._read_nlist_64(nOffsetSymTab);

                        nIndex = _nlist.n_strx;
                        nAddress = _nlist.n_value;

                        nOffsetSymTab += sizeof(XMACH_DEF::nlist_64);
                    } else {
                        XMACH_DEF::nlist _nlist = mach._read_nlist(nOffsetSymTab);

                        nIndex = _nlist.n_strx;
                        nAddress = _nlist.n_value;

                        nOffsetSymTab += sizeof(XMACH_DEF::nlist);
                    }

                    if (nAddress) {
                        QString sSymbol = mach._read_ansiString_safe(pBuffer, nBufferSize, nIndex, 256);
                        addSymbol(pState, nAddress, 0, 0, sSymbol);
                    }

                    if (nIndex == 0) {
                        break;
                    }
                }
            }

            {
                qint64 nOffsetFS = mach.getCommandRecordOffset(XMACH_DEF::S_LC_FUNCTION_STARTS, 0, &listCR);

                if (nOffsetFS != -1) {
                    XMACH_DEF::linkedit_data_command linkedit = mach._read_linkedit_data_command(nOffsetFS);
                    QList<XMACH::FUNCTION_RECORD> listFR = mach.getFunctionRecords(linkedit.dataoff, linkedit.datasize);

                    qint32 nNumberOfRecords = listFR.count();

                    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                        addSymbolOrUpdateFlags(pState, listFR.at(i).nFunctionAddress, 0, XSYMBOL_FLAG_FUNCTION);
                    }
                }
            }

            {
                QList<XMACH::EXPORT_RECORD> listExportRecords;

                {
                    qint64 nOffsetExports = mach.getCommandRecordOffset(XMACH_DEF::S_LC_DYLD_EXPORTS_TRIE, 0, &listCR);

                    if (nOffsetExports != -1) {
                        XMACH_DEF::linkedit_data_command linkedit = mach._read_linkedit_data_command(nOffsetExports);
                        mach.handleImport(linkedit.dataoff, 0, linkedit.datasize, &listExportRecords, "", pPdStruct);
                    }
                }

                if (!listExportRecords.count()) {
                    qint64 nOffsetExports = mach.getCommandRecordOffset(XMACH_DEF::S_LC_DYLD_INFO, 0, &listCR);

                    if (nOffsetExports != -1) {
                        XMACH_DEF::dyld_info_command dyldInfo = mach._read_dyld_info_command(nOffsetExports);

                        if (dyldInfo.export_size) {
                            mach.handleImport(dyldInfo.export_off, 0, dyldInfo.export_size, &listExportRecords, "", pPdStruct);
                        }
                    }
                }

                if (!listExportRecords.count()) {
                    qint64 nOffsetExports = mach.getCommandRecordOffset(XMACH_DEF::S_LC_DYLD_INFO_ONLY, 0, &listCR);

                    if (nOffsetExports != -1) {
                        XMACH_DEF::dyld_info_command dyldInfo = mach._read_dyld_info_command(nOffsetExports);

                        if (dyldInfo.export_size) {
                            mach.handleImport(dyldInfo.export_off, 0, dyldInfo.export_size, &listExportRecords, "", pPdStruct);
                        }
                    }
                }

                qint32 nNumberOfRecords = listExportRecords.count();

                for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                    XADDR nExportAddress = XBinary::offsetToAddress(&(pState->memoryMap), listExportRecords.at(i).nOffset);
                    quint32 nFlags = listExportRecords.at(i).nFlags;  // TODO
                    addSymbolOrUpdateFlags(pState, nExportAddress, 0, XSYMBOL_FLAG_EXPORT, listExportRecords.at(i).sName);
                }
            }

            if (pState->memoryMap.nEntryPointAddress != -1) {
                addSymbolOrUpdateFlags(pState, pState->memoryMap.nEntryPointAddress, 0, XSYMBOL_FLAG_FUNCTION | XSYMBOL_FLAG_ENTRYPOINT);
            }
        }
    } else if ((fileType == XBinary::FT_PE32) || (fileType == XBinary::FT_PE64)) {
        XPE pe(pState->pDevice, pState->bIsImage, pState->nModuleAddress);

        if (pe.isValid()) {
            pState->memoryMap = pe.getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);

            addSymbolOrUpdateFlags(pState, pState->memoryMap.nEntryPointAddress, 0, XSYMBOL_FLAG_FUNCTION | XSYMBOL_FLAG_ENTRYPOINT);

            XPE::EXPORT_HEADER exportHeader = pe.getExport(&(pState->memoryMap), false, pPdStruct);

            qint32 nNumberOfFunctions = exportHeader.listPositions.count();

            for (qint32 i = 0; i < nNumberOfFunctions; i++) {
                addSymbolOrUpdateFlags(pState, exportHeader.listPositions.at(i).nAddress, 0, XSYMBOL_FLAG_FUNCTION | XSYMBOL_FLAG_EXPORT,
                                       exportHeader.listPositions.at(i).sFunctionName);
            }
        }
    }

    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        XBinary binary(pState->pDevice, pState->bIsImage, pState->nModuleAddress);

        char *pMemory = 0;
        XBinary::_MEMORY_RECORD mr = {};
        qint32 nCurrentSegment = -1;

        while (XBinary::isPdStructNotCanceled(pPdStruct)) {
            qint32 nNumbersOfSymbols = pState->listSymbols.count();

            bool bContinue = false;

            qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
            XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

            for (qint32 i = 0; (i < nNumbersOfSymbols) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                if ((pState->listSymbols.at(i).nFlags & XSYMBOL_FLAG_FUNCTION) && (pState->listSymbols.at(i).nBranch == 0)) {
                    bContinue = true;
                    XSYMBOL function = pState->listSymbols.at(i);

                    if (function.nRegionIndex != nCurrentSegment) {
                        mr = pState->memoryMap.listRecords.at(function.nRegionIndex);

                        if (mr.nOffset != -1) {
                            if (pMemory) {
                                delete[] pMemory;
                            }

                            pMemory = new char[mr.nSize];

                            if (pMemory) {
                                // Correct size
                                mr.nSize = binary.read_array_process(mr.nOffset, pMemory, mr.nSize, pPdStruct);
                            }
                        }
                    }

                    qint32 nSize = mr.nSize - function.nRelOffset;

                    if (i + 1 < nNumbersOfSymbols) {
                        if (function.nRegionIndex == pState->listSymbols.at(i + 1).nRegionIndex) {
                            nSize = pState->listSymbols.at(i + 1).nRelOffset - function.nRelOffset;
                        }
                    }

                    if (pMemory) {
                        quint16 nBranch = ++(pState->nCurrentBranch);

                        _addCode(pState, &mr, pMemory, function.nRelOffset, nSize, nBranch, pPdStruct);
                        pState->listSymbols[i].nBranch = nBranch;  // mb TODO if _addCode
                    }
                }

                XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
            }

            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

            QSetIterator<XADDR> i(pState->stCodeTemp);
            while (i.hasNext() && XBinary::isPdStructNotCanceled(pPdStruct)) {
                XADDR nCurrentAddress = i.next();

                XBinary::_MEMORY_RECORD mrCurrent = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nCurrentAddress);

                if (mrCurrent.nSize) {
                    qint32 nIndex = _searchXRecordBySegmentRelOffset(&(pState->listRecords), mrCurrent.nIndex, nCurrentAddress - mrCurrent.nAddress, true);

                    if (nIndex == -1) {
                        addSymbolOrUpdateFlags(pState, nCurrentAddress, 0, XSYMBOL_FLAG_FUNCTION);  // TODO optimize
                    }
                }
            }

            pState->stCodeTemp.clear();

            if (!bContinue) {
                break;
            }
        }

        if (pMemory) {
            delete[] pMemory;
        }
    }

    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        // Update refs
        std::sort(pState->listRefs.begin(), pState->listRefs.end(), compareXREFINFO_location_ref);

        qint32 nNumberOfRecords = pState->listRefs.count();

        qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

        for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            XREFINFO record = pState->listRefs.at(i);

            if (record.nRegionIndexRef == (quint16)-1) {
                XADDR nRefAddress = record.nRelOffsetRef;

                XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nRefAddress);

                if (mr.nSize) {
                    pState->listRefs[i].nRegionIndexRef = mr.nIndex;
                    pState->listRefs[i].nRelOffsetRef = nRefAddress - mr.nAddress;
                }
            }

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        std::sort(pState->listRefs.begin(), pState->listRefs.end(), compareXREFINFO_location);
    }

    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
        // Update symbols
        QMap<quint16, XADDR> mapMaxAddress;

        qint32 nNumberOfRecords = pState->listRecords.count();

        qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

        for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            XRECORD record = pState->listRecords.at(i);

            XADDR nValue = record.nRelOffset + record.nSize;

            if (nValue > mapMaxAddress.value(record.nBranch)) {
                mapMaxAddress.insert(record.nBranch, nValue);
            }

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        qint32 nNumberOfSymbols = pState->listSymbols.count();

        for (qint32 i = 0; (i < nNumberOfSymbols) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            XSYMBOL symbol = pState->listSymbols.at(i);

            if (symbol.nBranch) {
                pState->listSymbols[i].nSize = mapMaxAddress.value(symbol.nBranch) - symbol.nRelOffset;
            }
        }
    }

    if (XBinary::isPdStructStopped(pPdStruct)) {
        pState->listSymbols.clear();
        pState->listRecords.clear();
        pState->listRefs.clear();
        pState->listStrings.clear();
    } else {
        pState->bIsAnalyzed = true;
        reloadView();
    }

    return true;
}

void XInfoDB::_addCode(STATE *pState, XBinary::_MEMORY_RECORD *pMemoryRecord, char *pMemory, XADDR nRelOffset, qint64 nSize, quint16 nBranch,
                       XBinary::PDSTRUCT *pPdStruct)
{
    XDisasmAbstract::DISASM_OPTIONS disasmOptions = {};
    disasmOptions.bNoStrings = true;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, pMemoryRecord->nSize);

    quint64 nTotalSize = qMin(pMemoryRecord->nSize, (qint64)(nRelOffset + nSize));

    XADDR nRefAddress = 0;

    for (quint64 i = nRelOffset; (i < nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        bool bStop = false;
        XRECORD dataRecord = {};
        XREFINFO refInfo = {};
        XADDR nRelOffsetSameSegment = -1;

        qint32 nDataSize = 0;

        qint32 nRefDataSize = 0;
        // if ((nTotalSize - nRelOffset) >= 4) {
        //     nRefAddress = *(quint32 *)(pMemory + i);

        //     if (XBinary::isAddressValid(&(pState->memoryMap), nRefAddress)) {
        //         nRefDataSize = 4;
        //     }
        // }
        if (pState->listRecords.count() >= 1000000) {
            break;
        }

        if (nRefDataSize) {
            dataRecord.nRelOffset = i;
            dataRecord.nRegionIndex = pMemoryRecord->nIndex;
            dataRecord.nSize = 4;
            dataRecord.nFlags |= XRECORD_FLAG_CODE | XRECORD_FLAG_ADDRREF;

            refInfo.nRelOffset = i;
            refInfo.nRegionIndex = pMemoryRecord->nIndex;
            refInfo.nRelOffsetRef = nRefAddress;
            refInfo.nRegionIndexRef = -1;
            refInfo.nFlags |= XREF_FLAG_MEMORY;
            refInfo.nSize = 4;

            nDataSize = 4;
        } else {
            XDisasmAbstract::DISASM_RESULT dr = pState->disasmCore.disAsm(pMemory + i, nTotalSize - i, pMemoryRecord->nAddress + i, disasmOptions);

            if (dr.bIsValid) {
                dataRecord.nRelOffset = i;
                dataRecord.nRegionIndex = pMemoryRecord->nIndex;
                dataRecord.nSize = dr.nSize;
                dataRecord.nFlags |= XRECORD_FLAG_CODE | XRECORD_FLAG_OPCODE;
                dataRecord.nBranch = nBranch;

                if ((dr.relType != XDisasmAbstract::RELTYPE_NONE) || (dr.memType != XDisasmAbstract::MEMTYPE_NONE) || dr.bIsCall || dr.bIsJmp || dr.bIsCondJmp ||
                    dr.bIsRet) {
                    refInfo.nRelOffset = i;
                    refInfo.nRegionIndex = pMemoryRecord->nIndex;
                    refInfo.nBranch = nBranch;

                    if (dr.bIsCall) refInfo.nFlags |= XREF_FLAG_CALL;
                    else if (dr.bIsJmp) refInfo.nFlags |= XREF_FLAG_JMP;
                    else if (dr.bIsCondJmp) refInfo.nFlags |= XREF_FLAG_JMP_COND;
                    else if (dr.bIsRet) refInfo.nFlags |= XREF_FLAG_RET;

                    if (dr.bIsCall || dr.bIsJmp || dr.bIsCondJmp) {
                        if (dr.relType != XDisasmAbstract::RELTYPE_NONE) {
                            pState->stCodeTemp.insert(dr.nXrefToRelative);
                        }
                    }
                }

                if (dr.relType != XDisasmAbstract::RELTYPE_NONE) {
                    refInfo.nFlags |= XREF_FLAG_REL;
                    refInfo.nSize = 0;

                    if ((dr.nXrefToRelative >= pMemoryRecord->nAddress) && (dr.nXrefToRelative < (pMemoryRecord->nAddress + pMemoryRecord->nSize))) {
                        refInfo.nRelOffsetRef = dr.nXrefToRelative - pMemoryRecord->nAddress;
                        refInfo.nRegionIndexRef = pMemoryRecord->nIndex;

                        if (refInfo.nRelOffsetRef > i) {
                            if (dr.relType == XDisasmAbstract::RELTYPE_JMP_COND) {
                                nRelOffsetSameSegment = refInfo.nRelOffsetRef;
                            } else if (dr.relType == XDisasmAbstract::RELTYPE_JMP) {
                                if ((refInfo.nRelOffsetRef - (nRelOffset + i)) < 128) {
                                    nRelOffsetSameSegment = refInfo.nRelOffsetRef;
                                }
                            }
                        }
                    } else {
                        refInfo.nRelOffsetRef = dr.nXrefToRelative;
                        refInfo.nRegionIndexRef = -1;
                    }
                }

                if (dr.memType != XDisasmAbstract::MEMTYPE_NONE) {
                    refInfo.nFlags |= XREF_FLAG_MEMORY;
                    refInfo.nSize = dr.nMemorySize;

                    if ((dr.nXrefToMemory >= pMemoryRecord->nAddress) && (dr.nXrefToMemory < (pMemoryRecord->nAddress + pMemoryRecord->nSize))) {
                        refInfo.nRelOffsetRef = dr.nXrefToMemory - pMemoryRecord->nAddress;
                        refInfo.nRegionIndexRef = pMemoryRecord->nIndex;
                    } else {
                        refInfo.nRelOffsetRef = dr.nXrefToMemory;
                        refInfo.nRegionIndexRef = -1;
                    }
                }

                nDataSize = dr.nSize;

                // if (dr.bIsJmp) {
                //     QString sTst = QString("%1 %2 %3").arg(XBinary::valueToHex(dr.nAddress), dr.sMnemonic, dr.sOperands);
                //     qDebug("%s", sTst.toLatin1().data());
                // }

                // TODO
                if (dr.bIsRet || dr.bIsJmp) {
                    bStop = true;
                }
            } else {
                bStop = true;
            }
        }

        if (dataRecord.nFlags) {
            if (_insertXRecord(&(pState->listRecords), dataRecord)) {
                if (refInfo.nFlags) {
                    _insertXRefinfo(&(pState->listRefs), refInfo);
                }
            } else {
                bStop = true;
            }
        } else {
            bStop = true;
        }

        i += nDataSize;

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);

        if (nRelOffsetSameSegment != (XADDR)-1) {
            _addCode(pState, pMemoryRecord, pMemory, nRelOffsetSameSegment, nSize - (nRelOffsetSameSegment - nRelOffset), nBranch, pPdStruct);
        }

        if (bStop) {
            break;
        }
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
}

bool XInfoDB::_isCode(STATE *pState, XBinary::_MEMORY_RECORD *pMemoryRecord, char *pMemory, XADDR nRelOffset, qint64 nSize)
{
    Q_UNUSED(pState)
    Q_UNUSED(pMemoryRecord)
    Q_UNUSED(pMemory)
    Q_UNUSED(nRelOffset)

    return true;
}

bool XInfoDB::addSymbol(STATE *pState, XADDR nAddress, quint32 nSize, quint16 nFlags, const QString &sSymbolName, quint16 nBranch)
{
    bool bResult = false;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nAddress);

    XSYMBOL symbol = {};
    symbol.nStringIndex = -1;

    if (mr.nSize) {
        symbol.nRegionIndex = mr.nIndex;
        symbol.nRelOffset = nAddress - mr.nAddress;
    } else {
        symbol.nRegionIndex = -1;
        symbol.nRelOffset = mr.nAddress;
    }

    symbol.nSize = nSize;
    symbol.nFlags = nFlags;
    symbol.nBranch = nBranch;

    if (sSymbolName != "") {
        pState->listStrings.append(sSymbolName);
        symbol.nStringIndex = pState->listStrings.size() - 1;
    }

    bResult = _insertXSymbol(&(pState->listSymbols), symbol);

    return bResult;
}

bool XInfoDB::addRefInfo(STATE *pState, XADDR nAddress, XADDR nAddressRef, quint32 nSize, quint16 nFlags, quint16 nBranch)
{
    bool bResult = false;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nAddress);
    XBinary::_MEMORY_RECORD mrRef = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nAddressRef);

    XREFINFO refInfo = {};

    if (mr.nSize) {
        refInfo.nRegionIndex = mr.nIndex;
        refInfo.nRelOffset = nAddress - mr.nAddress;
    } else {
        refInfo.nRegionIndex = -1;
        refInfo.nRelOffset = mr.nAddress;
    }

    if (mrRef.nSize) {
        refInfo.nRegionIndexRef = mrRef.nIndex;
        refInfo.nRelOffsetRef = nAddressRef - mrRef.nAddress;
    } else {
        refInfo.nRegionIndexRef = -1;
        refInfo.nRelOffsetRef = mrRef.nAddress;
    }

    refInfo.nSize = nSize;
    refInfo.nFlags = nFlags;
    refInfo.nBranch = nBranch;

    bResult = _insertXRefinfo(&(pState->listRefs), refInfo);

    return bResult;
}

bool XInfoDB::addRecord(STATE *pState, XADDR nAddress, quint16 nSize, quint16 nFlags, quint16 nBranch)
{
    bool bResult = false;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nAddress);

    XRECORD record = {};

    if (mr.nSize) {
        record.nRegionIndex = mr.nIndex;
        record.nRelOffset = nAddress - mr.nAddress;
    } else {
        record.nRegionIndex = -1;
        record.nRelOffset = mr.nAddress;
    }

    record.nSize = nSize;
    record.nFlags = nFlags;
    record.nBranch = nBranch;

    bResult = _insertXRecord(&(pState->listRecords), record);

    return bResult;
}

bool XInfoDB::updateSymbolFlags(STATE *pState, XADDR nAddress, quint16 nFlags)
{
    bool bResult = false;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nAddress);

    if (mr.nSize) {
        qint32 nIndex = _searchXSymbolBySegmentRelOffset(&(pState->listSymbols), mr.nIndex, nAddress - mr.nAddress);

        if (nIndex != -1) {
            pState->listSymbols[nIndex].nFlags |= nFlags;
            bResult = true;
        }
    }

    return bResult;
}

bool XInfoDB::addSymbolOrUpdateFlags(STATE *pState, XADDR nAddress, quint32 nSize, quint16 nFlags, const QString &sSymbolName)
{
    bool bResult = false;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nAddress);

    if (mr.nSize) {
        qint32 nIndex = _searchXSymbolBySegmentRelOffset(&(pState->listSymbols), mr.nIndex, nAddress - mr.nAddress);

        if (nIndex == -1) {
            bResult = addSymbol(pState, nAddress, nSize, nFlags, sSymbolName);
        } else {
            pState->listSymbols[nIndex].nFlags |= nFlags;
            bResult = true;
        }
    }

    return bResult;
}

void XInfoDB::dumpBookmarks()
{
#ifdef QT_DEBUG
    qint32 nNumberOfBookmarks = m_listBookmarks.count();

    for (qint32 i = 0; i < nNumberOfBookmarks; i++) {
        BOOKMARKRECORD bookmark = m_listBookmarks.at(i);

        QString sDebugString =
            QString("%1 %2 %3 %4 %5 %6 %7 %8 %9")
                .arg(bookmark.sUUID, XBinary::valueToHex(bookmark.nLocation), QString::number(bookmark.locationType), XBinary::valueToHex(bookmark.nSize),
                     bookmark.sColorText, bookmark.sColorBackground, bookmark.sTemplate, bookmark.sComment, bookmark.bIsUser ? "true" : "false");

        qDebug("%s", sDebugString.toUtf8().data());
    }
#endif
}

void XInfoDB::dumpSymbols(XBinary::FT fileType)
{
#ifdef QT_DEBUG
    XInfoDB::STATE *pState = getState(fileType);

    qint32 nNumberOfSymbols = pState->listSymbols.count();

    for (qint32 i = 0; i < nNumberOfSymbols; i++) {
        XSYMBOL symbol = pState->listSymbols.at(i);
        QString sSymbolName = "";

        if (symbol.nStringIndex != (quint16)-1) {
            sSymbolName = m_mapProfiles.value(XBinary::FT_MACHO64)->listStrings.at(symbol.nStringIndex);
        }

        QString sDebugString = QString("%1 %2 %3 %4 %5")
                                   .arg(XBinary::valueToHex(symbol.nRelOffset), XBinary::valueToHex(symbol.nSize), XBinary::valueToHex(symbol.nRegionIndex),
                                        XBinary::valueToHex(symbol.nFlags), sSymbolName);

        qDebug("%s", sDebugString.toUtf8().data());
    }
#endif
}

void XInfoDB::dumpRecords(XBinary::FT fileType)
{
#ifdef QT_DEBUG
    XInfoDB::STATE *pState = getState(fileType);

    qint32 nNumberOfRecords = pState->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XRECORD record = pState->listRecords.at(i);

        QString sDebugString = QString("%1 %2 %3 %4 %5")
                                   .arg(XBinary::valueToHex(record.nRelOffset), XBinary::valueToHex(record.nSize), XBinary::valueToHex(record.nRegionIndex),
                                        XBinary::valueToHex(record.nBranch), XBinary::valueToHex(record.nFlags));

        qDebug("%s", sDebugString.toUtf8().data());
    }
#endif
}

void XInfoDB::dumpRefs(XBinary::FT fileType)
{
#ifdef QT_DEBUG
    XInfoDB::STATE *pState = getState(fileType);

    qint32 nNumberOfRefs = pState->listRefs.count();

    for (qint32 i = 0; i < nNumberOfRefs; i++) {
        XREFINFO refInfo = pState->listRefs.at(i);

        QString sDebugString = QString("%1 %2 %3 %4 %5 %6 %7")
                                   .arg(XBinary::valueToHex(refInfo.nRelOffset), XBinary::valueToHex(refInfo.nRelOffsetRef), XBinary::valueToHex(refInfo.nRegionIndex),
                                        XBinary::valueToHex(refInfo.nRegionIndexRef), XBinary::valueToHex(refInfo.nSize), XBinary::valueToHex(refInfo.nFlags),
                                        XBinary::valueToHex(refInfo.nBranch));

        qDebug("%s", sDebugString.toUtf8().data());
    }
#endif
}

void XInfoDB::dumpShowRecords(XBinary::FT fileType)
{
#ifdef QT_DEBUG
    XInfoDB::STATE *pState = getState(fileType);
    XBinary binary(pState->pDevice);
    XDisasmAbstract::DISASM_OPTIONS disasmOptions = {};

    qint32 nNumberOfRecords = pState->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XRECORD record = pState->listRecords.at(i);

        XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByIndex(&(pState->memoryMap), record.nRegionIndex);
        qint64 nOffset = mr.nOffset + record.nRelOffset;
        XADDR nAddress = mr.nAddress + record.nRelOffset;

        QByteArray baData = binary.read_array(nOffset, record.nSize);

        QString sShowRecord = getShowString(pState, record, disasmOptions);

        QString sDebugString = QString("%1: %2 %3").arg(XBinary::valueToHex(nAddress), baData.toHex(), sShowRecord);

        qDebug("%s", sDebugString.toUtf8().data());
    }
#endif
}

QString XInfoDB::getShowString(STATE *pState, const XRECORD &record, const XDisasmAbstract::DISASM_OPTIONS &disasmOptions)
{
    QString sResult = "";

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByIndex(&(pState->memoryMap), record.nRegionIndex);
    qint64 nOffset = mr.nOffset + record.nRelOffset;
    XADDR nAddress = mr.nAddress + record.nRelOffset;

    QByteArray baData = XBinary::read_array(pState->pDevice, nOffset, record.nSize);

    if (record.nFlags & XRECORD_FLAG_OPCODE) {
        XDisasmAbstract::DISASM_RESULT disasmResult = pState->disasmCore.disAsm(baData.data(), baData.size(), nAddress, disasmOptions);

        sResult = disasmResult.sMnemonic;

        if (disasmResult.sOperands != "") {
            QString sOperands = disasmResult.sOperands;
            QString sReplace;
            QString sSymbol;

            if (disasmResult.relType) {
                sReplace = pState->disasmCore.getNumberString(disasmResult.nXrefToRelative);
                sSymbol = _getSymbolStringByAddress(pState, disasmResult.nXrefToRelative);
            }

            if (disasmResult.memType) {
                sReplace = pState->disasmCore.getNumberString(disasmResult.nXrefToMemory);
                sSymbol = _getSymbolStringByAddress(pState, disasmResult.nXrefToMemory);
            }

            if ((sReplace != "") && (sSymbol != "")) {
                sOperands = sOperands.replace(sReplace, sSymbol, Qt::CaseInsensitive);
            }

            sResult += " " + sOperands;
        }

    } else {
    }

    return sResult;
}

QString XInfoDB::_getSymbolStringBySegmentRelOffset(STATE *pState, quint16 nRegionIndex, XADDR nRelOffset)
{
    QString sResult;

    qint32 nIndex = _searchXSymbolBySegmentRelOffset(&(pState->listSymbols), nRegionIndex, nRelOffset);

    if (nIndex != -1) {
        XSYMBOL xsymbol = pState->listSymbols.at(nIndex);

        if (xsymbol.nStringIndex != (quint16)-1) {
            sResult = pState->listStrings.at(xsymbol.nStringIndex);
        } else {
            sResult = QString("loc_%1").arg(XBinary::valueToHexEx(pState->memoryMap.listRecords.at(nRegionIndex).nAddress + nRelOffset));
        }
    }

    return sResult;
}

QString XInfoDB::_getSymbolStringByAddress(STATE *pState, XADDR nAddress)
{
    QString sResult;

    XBinary::_MEMORY_RECORD memoryRecord = XBinary::getMemoryRecordByAddress(&(pState->memoryMap), nAddress);

    if (memoryRecord.nSize) {
        sResult = _getSymbolStringBySegmentRelOffset(pState, memoryRecord.nIndex, nAddress - memoryRecord.nAddress);
    }

    return sResult;
}

void XInfoDB::setData(QIODevice *pDevice, XBinary::FT fileType)
{
    addMode(pDevice, fileType);
}

XBinary::FT XInfoDB::addMode(QIODevice *pDevice, XBinary::FT fileType)
{
    XBinary::FT result = XBinary::FT_UNKNOWN;

    if (fileType == XBinary::FT_MACHO64) {
        result = fileType;
    } else if ((fileType == XBinary::FT_PE32) || (fileType == XBinary::FT_PE64)) {
        result = fileType;
    }

    if (!m_mapProfiles.contains(result)) {
        XInfoDB::STATE *pState = new STATE;
        pState->bIsAnalyzed = false;
        pState->nCurrentBranch = 0;
        pState->pDevice = pDevice;
        pState->bIsImage = false;     // TODO
        pState->nModuleAddress = -1;  // TODO

        if (result != XBinary::FT_UNKNOWN) {
            pState->memoryMap = XFormats::getMemoryMap(result, XBinary::MAPMODE_UNKNOWN, pDevice, pState->bIsImage, pState->nModuleAddress, nullptr);
        } else {
            pState->memoryMap = XFormats::getMemoryMap(XBinary::FT_BINARY, XBinary::MAPMODE_UNKNOWN, pDevice, pState->bIsImage, pState->nModuleAddress, nullptr);
        }

        pState->disasmCore.setMode(XBinary::getDisasmMode(&pState->memoryMap));

        m_mapProfiles.insert(result, pState);
    }

    return result;
}

qint64 XInfoDB::getOffset(STATE *pState, quint16 nRegionIndex, XADDR nRelOffset)
{
    qint64 nResult = -1;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByIndex(&(pState->memoryMap), nRegionIndex);

    if ((mr.nSize) && (mr.nAddress != -1)) {
        nResult = mr.nOffset + nRelOffset;
    }

    return nResult;
}

XADDR XInfoDB::getAddress(STATE *pState, quint16 nRegionIndex, XADDR nRelOffset)
{
    XADDR nResult = -1;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByIndex(&(pState->memoryMap), nRegionIndex);

    if ((mr.nSize) && (mr.nOffset != -1)) {
        nResult = mr.nAddress + nRelOffset;
    }

    return nResult;
}

qint32 XInfoDB::_searchXRecordByAddress(XBinary::_MEMORY_MAP *pMemoryMap, QVector<XRECORD> *pListRecords, XADDR nAddress, bool bInRecord)
{
    qint32 nResult = -1;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(pMemoryMap, nAddress);

    if (mr.nSize) {
        nResult = _searchXRecordBySegmentRelOffset(pListRecords, mr.nIndex, nAddress - mr.nAddress, bInRecord);
    }

    return nResult;
}

qint32 XInfoDB::_searchXRecordByAddress(STATE *pState, XADDR nAddress, bool bInRecord)
{
    return _searchXRecordByAddress(&(pState->memoryMap), &(pState->listRecords), nAddress, bInRecord);
}

qint32 XInfoDB::_searchXSymbolByAddress(XBinary::_MEMORY_MAP *pMemoryMap, QVector<XSYMBOL> *pListSymbols, XADDR nAddress)
{
    qint32 nResult = -1;

    XBinary::_MEMORY_RECORD mr = XBinary::getMemoryRecordByAddress(pMemoryMap, nAddress);

    if (mr.nSize) {
        nResult = _searchXSymbolBySegmentRelOffset(pListSymbols, mr.nIndex, nAddress - mr.nAddress);
    }

    return nResult;
}

qint32 XInfoDB::_searchXSymbolBySegmentRelOffset(QVector<XSYMBOL> *pListSymbols, quint16 nRegionIndex, XADDR nRelOffset)
{
    XSYMBOL searchSymbol = {};
    searchSymbol.nRegionIndex = nRegionIndex;
    searchSymbol.nRelOffset = nRelOffset;

    QVector<XSYMBOL>::iterator it = std::lower_bound(pListSymbols->begin(), pListSymbols->end(), searchSymbol, compareXSYMBOL_location);

    if (it != pListSymbols->end() && it->nRegionIndex == nRegionIndex && it->nRelOffset == nRelOffset) {
        return it - pListSymbols->begin();
    }

    return -1;  // Not found
}

bool XInfoDB::_insertXSymbol(QVector<XSYMBOL> *pListSymbols, const XSYMBOL &symbol)
{
    QVector<XSYMBOL>::iterator it = std::lower_bound(pListSymbols->begin(), pListSymbols->end(), symbol, compareXSYMBOL_location);

    if (it != pListSymbols->end()) {
        if (it->nRegionIndex == symbol.nRegionIndex && it->nRelOffset == symbol.nRelOffset) {
            return false;
        }
    }

    pListSymbols->insert(it, symbol);

    return true;
}

qint32 XInfoDB::_searchXRefinfoBySegmentRelOffset(QVector<XREFINFO> *pListRefs, quint16 nRegionIndex, XADDR nRelOffset)
{
    XREFINFO searchInfo = {};
    searchInfo.nRegionIndex = nRegionIndex;
    searchInfo.nRelOffset = nRelOffset;

    QVector<XREFINFO>::iterator it = std::lower_bound(pListRefs->begin(), pListRefs->end(), searchInfo, compareXREFINFO_location);

    if (it != pListRefs->end() && it->nRegionIndex == nRegionIndex && it->nRelOffset == nRelOffset) {
        return it - pListRefs->begin();
    }

    return -1;
}

bool XInfoDB::_insertXRefinfo(QVector<XREFINFO> *pListRefs, const XREFINFO &refinfo)
{
    QVector<XREFINFO>::iterator it = std::lower_bound(pListRefs->begin(), pListRefs->end(), refinfo, compareXREFINFO_location);

    if (it != pListRefs->end()) {
        if (it->nRegionIndex == refinfo.nRegionIndex && it->nRelOffset == refinfo.nRelOffset) {
            return false;
        }
    }

    pListRefs->insert(it, refinfo);

    return true;
}

qint32 XInfoDB::_searchXRecordBySegmentRelOffset(QVector<XRECORD> *pListRecords, quint16 nRegionIndex, XADDR nRelOffset, bool bInRecord)
{
    XRECORD searchRecord = {};
    searchRecord.nRegionIndex = nRegionIndex;
    searchRecord.nRelOffset = nRelOffset;

    QVector<XRECORD>::iterator it = std::lower_bound(pListRecords->begin(), pListRecords->end(), searchRecord, compareXRECORD_location);

    if (it != pListRecords->end() && it->nRegionIndex == nRegionIndex && it->nRelOffset == nRelOffset) {
        return it - pListRecords->begin();
    }

    if (bInRecord) {
        if (it != pListRecords->begin()) {
            it--;
            if (it->nRegionIndex == nRegionIndex && (it->nRelOffset <= nRelOffset) && (nRelOffset < (it->nRelOffset + it->nSize))) {
                return it - pListRecords->begin();
            }
        }
    }

    return -1;
}

bool XInfoDB::_insertXRecord(QVector<XRECORD> *pListSymbols, const XRECORD &record)
{
    QVector<XRECORD>::iterator it = std::lower_bound(pListSymbols->begin(), pListSymbols->end(), record, compareXRECORD_location);

    if (it != pListSymbols->end()) {
        if (it->nRegionIndex == record.nRegionIndex && it->nRelOffset == record.nRelOffset) {
            return false;
        }
    }

    pListSymbols->insert(it, record);
    return true;
}

#ifdef QT_SQL_LIB
bool XInfoDB::_addShowRecord_prepare(QSqlQuery *pQuery)
{
    // return pQuery->prepare(QString("INSERT INTO %1 (ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS) "
    //                                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)")
    //                            .arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));
    return false;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::_isShowRecordPresent_prepare1(QSqlQuery *pQuery)
{
    // return pQuery->prepare(QString("SELECT ADDRESS FROM %1 WHERE ADDRESS = ?").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));
    return false;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::_isShowRecordPresent_prepare2(QSqlQuery *pQuery)
{
    // return pQuery->prepare(QString("SELECT ADDRESS FROM %1 WHERE (ADDRESS >= ?) AND (ADDRESS < ?)").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));
    return false;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::_isShowRecordPresent_bind1(QSqlQuery *pQuery, XADDR nAddress)
{
    bool bResult = false;

    pQuery->bindValue(0, nAddress);

    if (querySQL(pQuery, false)) {
        bResult = pQuery->next();
    }

    return bResult;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::_isShowRecordPresent_bind(QSqlQuery *pQuery1, QSqlQuery *pQuery2, XADDR nAddress, qint64 nSize)
{
    bool bResult = false;

    if (nSize <= 1) {
        pQuery1->bindValue(0, nAddress);

        if (querySQL(pQuery1, false)) {
            bResult = pQuery1->next();
        }
    } else {
        pQuery2->bindValue(0, nAddress);
        pQuery2->bindValue(1, nAddress + nSize);

        if (querySQL(pQuery2, false)) {
            bResult = pQuery2->next();
        }
    }

    return bResult;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::_addRelRecord_prepare(QSqlQuery *pQuery)
{
    // return pQuery->prepare(QString("INSERT INTO %1 (ADDRESS, RELTYPE, XREFTORELATIVE, MEMTYPE, XREFTOMEMORY, MEMORYSIZE, DBSTATUS) "
    //                                "VALUES (?, ?, ?, ?, ?, ?, ?)")
    //                            .arg(s_sql_tableName[DBTABLE_RELATIVS]));
    return false;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::_isShowRecordPresent(QSqlQuery *pQuery, XADDR nAddress, qint64 nSize)
{
    bool bResult = false;

    // if (nSize <= 1) {
    //     pQuery->prepare(QString("SELECT ADDRESS FROM %1 WHERE ADDRESS = ?").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));
    //     pQuery->bindValue(0, nAddress);
    // } else {
    //     pQuery->prepare(QString("SELECT ADDRESS FROM %1 WHERE (ADDRESS >= ?) AND (ADDRESS < ?)").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));
    //     pQuery->bindValue(0, nAddress);
    //     pQuery->bindValue(1, nAddress + nSize);
    // }

    // if (querySQL(pQuery, false)) {
    //     bResult = pQuery->next();
    // }

    return bResult;
}
#endif

void XInfoDB::_completeDbAnalyze()
{
    // TODO
}

#ifdef QT_SQL_LIB
quint64 XInfoDB::_getBranchNumber()
{
    quint64 nResult = 0;

    // QSqlQuery query(g_dataBase);

    // querySQL(&query, QString("SELECT MAX(BRANCH) + 1 FROM %1").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]), false);

    // if (query.next()) {
    //     nResult = query.value(0).toULongLong();
    // }

    // if (nResult == 0) {
    //     nResult = 1;
    // }

    return nResult;
}
#endif

bool XInfoDB::_incShowRecordRefFrom(XADDR nAddress)
{
    Q_UNUSED(nAddress)

    bool bResult = false;

    // #ifdef QT_SQL_LIB
    //     QSqlQuery query(g_dataBase);

    //     bResult = querySQL(&query, QString("UPDATE %1 SET REFFROM=REFFROM+1 WHERE ADDRESS=%2").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)),
    //     true);
    // #else
    //     Q_UNUSED(nAddress)
    // #endif

    return bResult;
}

bool XInfoDB::_removeAnalyze(XADDR nAddress, qint64 nSize)
{
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)

    bool bResult = false;

    // #ifdef QT_SQL_LIB
    //     QSqlQuery query(g_dataBase);

    //     bResult = querySQL(&query,
    //                        QString("DELETE FROM %1 WHERE ADDRESS >= %2 AND ADDRESS < %3")
    //                            .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress), QString::number(nAddress + nSize)),
    //                        true);

    //     // TODO REMOVE XREFS
    // #else
    //     Q_UNUSED(nAddress)
    //     Q_UNUSED(nSize)
    // #endif

    return bResult;
}

void XInfoDB::_clearAnalyze()
{
    // #ifdef QT_SQL_LIB
    //     QSqlQuery query(g_dataBase);

    //     querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]), true);
    //     querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_FUNCTIONS]), true);
    //     querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_RELATIVS]), true);
    //     querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_BOOKMARKS]), true);
    //     querySQL(&query, QString("DELETE FROM %1 WHERE SYMSOURCE <> '%2'").arg(s_sql_tableName[DBTABLE_SYMBOLS], QString::number(SS_FILE)), true);
    // #endif
}

bool XInfoDB::_setArray(XADDR nAddress, qint64 nSize)
{
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)

    bool bResult = false;

#ifdef QT_SQL_LIB
    // TODO
#endif

    return bResult;
}

bool XInfoDB::_addFunction(XADDR nAddress, qint64 nSize, const QString &sName)
{
    bool bResult = false;

    return bResult;
}

void XInfoDB::updateFunctionSize(XADDR nAddress, qint64 nSize)
{
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)
}

QString XInfoDB::_addBookmarkRecord(const BOOKMARKRECORD &record)
{
    m_listBookmarks.append(record);

    return record.sUUID;
}

bool XInfoDB::_removeBookmarkRecord(const QString &sUUID)
{
    bool bResult = false;

    qint32 nNumberOfBookmarks = m_listBookmarks.size();

    for (qint32 i = 0; i < nNumberOfBookmarks; i++) {
        if (m_listBookmarks.at(i).sUUID == sUUID) {
            m_listBookmarks.removeAt(i);
            bResult = true;
            break;
        }
    }

    return bResult;
}

QString XInfoDB::addBookmarkRecord(const BOOKMARKRECORD &record)
{
    QString sResult = _addBookmarkRecord(record);

    if (sResult != "") {
        setDatabaseChanged(true);
    }

    return sResult;
}

bool XInfoDB::removeBookmarkRecord(const QString &sUUID)
{
    bool bResult = _removeBookmarkRecord(sUUID);

    if (bResult) {
        setDatabaseChanged(true);
    }

    return bResult;
}

QVector<XInfoDB::BOOKMARKRECORD> *XInfoDB::getBookmarkRecords()
{
    return &m_listBookmarks;
}

QVector<XInfoDB::BOOKMARKRECORD> XInfoDB::getBookmarkRecords(quint64 nLocation, XBinary::LT locationType, qint64 nSize, XBinary::PDSTRUCT *pPdStruct)
{
    QVector<XInfoDB::BOOKMARKRECORD> listResult;

    qint32 nNumberOfRecords = m_listBookmarks.size();
    for (int i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const BOOKMARKRECORD &record = m_listBookmarks.at(i);

        bool bMatch = true;

        quint64 recordEnd = record.nLocation + record.nSize - 1;
        quint64 regionEnd = nLocation + nSize - 1;

        bool bOverlap = (record.nLocation <= regionEnd) && (recordEnd >= nLocation);

        if (!bOverlap) {
            bMatch = false;
        }

        if (bMatch) {
            listResult.append(record);
        }
    }

    return listResult;
}

void XInfoDB::updateBookmarkRecord(BOOKMARKRECORD &record)
{
    qint32 nNumberOfBookmarks = m_listBookmarks.size();

    for (int i = 0; i < nNumberOfBookmarks; i++) {
        if (m_listBookmarks.at(i).sUUID == record.sUUID) {
            m_listBookmarks[i] = record;
            setDatabaseChanged(true);
            break;
        }
    }
}

void XInfoDB::updateBookmarkRecordColorBackground(const QString &sUUID, const QString &sColorBackground)
{
    qint32 nNumberOfBookmarks = m_listBookmarks.size();

    for (int i = 0; i < nNumberOfBookmarks; i++) {
        if (m_listBookmarks.at(i).sUUID == sUUID) {
            m_listBookmarks[i].sColorBackground = sColorBackground;
            setDatabaseChanged(true);
            break;
        }
    }
}

void XInfoDB::updateBookmarkRecordComment(const QString &sUUID, const QString &sComment)
{
    qint32 nNumberOfBookmarks = m_listBookmarks.size();

    for (int i = 0; i < nNumberOfBookmarks; i++) {
        if (m_listBookmarks.at(i).sUUID == sUUID) {
            m_listBookmarks[i].sComment = sComment;
            setDatabaseChanged(true);
            break;
        }
    }
}

QList<XADDR> XInfoDB::getShowRecordRelAddresses(XDisasmAbstract::RELTYPE relType, DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(relType)
    Q_UNUSED(dbstatus)
    Q_UNUSED(pPdStruct)

    QList<XADDR> listResult;
    // #ifdef QT_SQL_LIB
    //     QSqlQuery query(g_dataBase);

    //     QString sSQL;

    //     if (relType == XDisasmAbstract::RELTYPE_ALL) {
    //         sSQL =
    //             QString("SELECT DISTINCT XREFTORELATIVE FROM %1 WHERE RELTYPE != %2").arg(s_sql_tableName[DBTABLE_RELATIVS],
    //             QString::number(XDisasmAbstract::RELTYPE_NONE));
    //     } else if (relType == XDisasmAbstract::RELTYPE_JMP) {
    //         sSQL = QString("SELECT DISTINCT XREFTORELATIVE FROM %1 WHERE RELTYPE IN(%2, %3, %4)")
    //                    .arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(XDisasmAbstract::RELTYPE_JMP), QString::number(XDisasmAbstract::RELTYPE_JMP_COND),
    //                         QString::number(XDisasmAbstract::RELTYPE_JMP_UNCOND));
    //     } else {
    //         sSQL = QString("SELECT DISTINCT XREFTORELATIVE FROM %1 WHERE RELTYPE = %2").arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(relType));
    //     }

    //     if (dbstatus != DBSTATUS_NONE) {
    //         sSQL += QString(" AND DBSTATUS = '%1'").arg(dbstatus);
    //     }

    //     querySQL(&query, sSQL, false);

    //     while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct))) {
    //         XADDR nAddress = query.value(0).toULongLong();

    //         listResult.append(nAddress);
    //     }
    // #else
    //     Q_UNUSED(relType)
    //     Q_UNUSED(dbstatus)
    //     Q_UNUSED(pPdStruct)
    // #endif
    return listResult;
}

QList<XBinary::ADDRESSSIZE> XInfoDB::getShowRecordMemoryVariables(DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct)
{
    QList<XBinary::ADDRESSSIZE> listResult;
    // #ifdef QT_SQL_LIB
    //     QSqlQuery query(g_dataBase);

    //     QString sSQL = QString("SELECT XREFTOMEMORY, MAX(MEMORYSIZE) FROM %1 WHERE MEMTYPE <> 0").arg(s_sql_tableName[DBTABLE_RELATIVS]);

    //     if (dbstatus != DBSTATUS_NONE) {
    //         sSQL += QString(" AND DBSTATUS = '%1'").arg(dbstatus);
    //     }

    //     sSQL += "GROUP BY XREFTOMEMORY";

    //     querySQL(&query, sSQL, false);

    //     while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct))) {
    //         XBinary::ADDRESSSIZE record = {};
    //         record.nAddress = query.value(0).toULongLong();
    //         record.nSize = query.value(1).toLongLong();

    //         listResult.append(record);
    //     }
    // #else
    //     Q_UNUSED(dbstatus)
    //     Q_UNUSED(pPdStruct)
    // #endif
    return listResult;
}

QList<XBinary::ADDRESSSIZE> XInfoDB::getBranches(DBSTATUS dbstatus, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(dbstatus)
    Q_UNUSED(pPdStruct)

    QList<XBinary::ADDRESSSIZE> listResult;
    // #ifdef QT_SQL_LIB
    //     QSqlQuery query(g_dataBase);

    //     QString sSQL = QString("SELECT MIN(ADDRESS), MAX(ADDRESS+SIZE) FROM %1 WHERE BRANCH > 0").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]);

    //     if (dbstatus != DBSTATUS_NONE) {
    //         sSQL += QString(" AND DBSTATUS = '%1'").arg(dbstatus);
    //     }

    //     sSQL += "GROUP BY BRANCH ORDER BY ADDRESS";

    //     querySQL(&query, sSQL, false);

    //     while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct))) {
    //         XBinary::ADDRESSSIZE record = {};
    //         record.nAddress = query.value(0).toULongLong();
    //         record.nSize = query.value(1).toULongLong() - record.nAddress;

    //         listResult.append(record);
    //     }
    // #else
    //     Q_UNUSED(dbstatus)
    //     Q_UNUSED(pPdStruct)
    // #endif
    return listResult;
}

QList<XADDR> XInfoDB::getExportSymbolAddresses(XBinary::PDSTRUCT *pPdStruct)
{
    QList<XADDR> listResult;

    return listResult;
}

QList<XADDR> XInfoDB::getImportSymbolAddresses(XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<XADDR> listResult;

    return listResult;
}

QList<XADDR> XInfoDB::getTLSSymbolAddresses(XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<XADDR> listResult;

    return listResult;
}

QList<XADDR> XInfoDB::getFunctionAddresses(XBinary::PDSTRUCT *pPdStruct)
{
    QList<XADDR> listResult;

    return listResult;
}

bool XInfoDB::isAddressHasRefFrom(XADDR nAddress)
{
    bool bResult = false;

    return bResult;
}

bool XInfoDB::isAnalyzedRegionVirtual(XADDR nAddress, qint64 nSize)
{
    bool bResult = false;
    return bResult;
}

bool XInfoDB::loadDbFromFile(QIODevice *pDevice, const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlDatabase dataBase = QSqlDatabase::addDatabase("QSQLITE", "local_db");
    dataBase.setDatabaseName(sDBFileName);

    if (dataBase.open()) {
        QSqlQuery query(dataBase);

        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            qint32 nNumberOfRecords = 0;

            querySQL(&query, QString("SELECT COUNT(*) FROM BOOKMARKS"), false);

            if (query.next()) {
                nNumberOfRecords = query.value(0).toInt();
            }

            if (nNumberOfRecords > 0) {
                m_listBookmarks.clear();

                querySQL(&query, QString("SELECT UUID, LOCATION, LOCTYPE, LOCSIZE, TEXTCOLOR, BACKGROUNDCOLOR, TEMPLATE, COMMENT, ISUSER FROM BOOKMARKS"), false);

                while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct)) {
                    BOOKMARKRECORD record = {};
                    record.sUUID = query.value(0).toString();
                    record.nLocation = query.value(1).toULongLong();
                    record.locationType = (XBinary::LT)query.value(2).toInt();
                    record.nSize = query.value(3).toLongLong();
                    record.sColorText = query.value(4).toString();
                    record.sColorBackground = query.value(5).toString();
                    record.sTemplate = query.value(6).toString();
                    record.sComment = query.value(7).toString();
                    record.bIsUser = query.value(8).toBool();

                    _addBookmarkRecord(record);
                }
            }
        }

        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            QList<XBinary::FT> listKeys;

            querySQL(&query,
                     QString("SELECT DISTINCT FILETYPE FROM SYMBOLS "
                             "UNION "
                             "SELECT DISTINCT FILETYPE FROM REFINFO "
                             "UNION "
                             "SELECT DISTINCT FILETYPE FROM RECORDS"),
                     false);

            while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct)) {
                XBinary::FT fileType = (XBinary::FT)query.value(0).toULongLong();

                if (addMode(pDevice, fileType) == fileType) {
                    listKeys.append(fileType);
                }
            }

            qint32 nNumberOfKeys = listKeys.count();

            for (int i = 0; (i < nNumberOfKeys) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                STATE *pState = m_mapProfiles.value(listKeys.at(i));

                if (pState) {
                    pState->listStrings.clear();
                    pState->listSymbols.clear();

                    querySQL(&query, QString("SELECT FILETYPE, ADDRESS, SIZE, NAME, FLAGS, BRANCH FROM SYMBOLS WHERE FILETYPE = %1").arg(listKeys.at(i)), false);

                    while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct)) {
                        XADDR nAddress = query.value(1).toULongLong();
                        quint32 nSize = query.value(2).toULongLong();
                        QString sName = query.value(3).toString();
                        quint16 nFlags = query.value(4).toULongLong();
                        quint16 nBranch = query.value(5).toULongLong();

                        addSymbol(pState, nAddress, nSize, nFlags, sName, nBranch);
                    }

                    pState->listRefs.clear();

                    querySQL(&query, QString("SELECT FILETYPE, ADDRESS, REFADDRESS, SIZE, FLAGS, BRANCH FROM REFINFO WHERE FILETYPE = %1").arg(listKeys.at(i)), false);

                    while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct)) {
                        XADDR nAddress = query.value(1).toULongLong();
                        XADDR nRefAddress = query.value(2).toULongLong();
                        quint32 nSize = query.value(3).toULongLong();
                        quint16 nFlags = query.value(4).toULongLong();
                        quint16 nBranch = query.value(5).toULongLong();

                        addRefInfo(pState, nAddress, nRefAddress, nSize, nFlags, nBranch);
                    }

                    pState->listRecords.clear();

                    querySQL(&query, QString("SELECT FILETYPE, ADDRESS, SIZE, FLAGS, BRANCH BRANCH FROM RECORDS WHERE FILETYPE = %1").arg(listKeys.at(i)), false);

                    while (query.next() && XBinary::isPdStructNotCanceled(pPdStruct)) {
                        XADDR nAddress = query.value(1).toULongLong();
                        quint32 nSize = query.value(2).toULongLong();
                        quint16 nFlags = query.value(3).toULongLong();
                        quint16 nBranch = query.value(4).toULongLong();

                        addRecord(pState, nAddress, nSize, nFlags, nBranch);
                    }
                }
            }
        }

        dataBase.close();

        // if (bResult) {
        //     setDatabaseChanged(false);
        // }
    }

    dataBase = QSqlDatabase();
    QSqlDatabase::removeDatabase("local_db");
#else
    Q_UNUSED(sDBFileName)
    Q_UNUSED(pPdStruct)
#endif
    return bResult;
}

bool XInfoDB::saveDbToFile(const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlDatabase dataBase = QSqlDatabase::addDatabase("QSQLITE", "local_db");
    dataBase.setDatabaseName(sDBFileName);

    if (dataBase.open()) {
        createTable(&dataBase, DBTABLE_BOOKMARKS);
        createTable(&dataBase, DBTABLE_SYMBOLS);
        createTable(&dataBase, DBTABLE_RECORDS);
        createTable(&dataBase, DBTABLE_REFINFO);

        QList<XBinary::FT> listKeys = m_mapProfiles.keys();

        QSqlQuery query(dataBase);

        dataBase.transaction();

        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            querySQL(&query, QString("DELETE FROM BOOKMARKS"), true);

            qint32 nNumberOfRecords = m_listBookmarks.count();

            query.prepare(
                "INSERT OR REPLACE INTO BOOKMARKS (UUID, LOCATION, LOCTYPE, LOCSIZE, TEXTCOLOR, BACKGROUNDCOLOR, TEMPLATE, COMMENT, ISUSER) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
            for (int i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                const BOOKMARKRECORD &record = m_listBookmarks.at(i);

                query.bindValue(0, record.sUUID);
                query.bindValue(1, record.nLocation);
                query.bindValue(2, record.locationType);
                query.bindValue(3, record.nSize);
                query.bindValue(4, record.sColorText);
                query.bindValue(5, record.sColorBackground);
                query.bindValue(6, record.sTemplate);
                query.bindValue(7, record.sComment);
                query.bindValue(8, record.bIsUser);

                bResult = querySQL(&query, true);
            }
        }

        if (XBinary::isPdStructNotCanceled(pPdStruct)) {
            qint32 nNumberOfKeys = listKeys.count();

            for (int i = 0; (i < nNumberOfKeys) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                STATE *pState = m_mapProfiles.value(listKeys.at(i));

                if (pState) {
                    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
                        querySQL(&query, QString("DELETE FROM SYMBOLS WHERE FILETYPE = %1").arg(listKeys.at(i)), true);

                        qint32 nNumberOfRecords = pState->listSymbols.count();

                        query.prepare(
                            "INSERT OR REPLACE INTO SYMBOLS (FILETYPE, ADDRESS, SIZE, NAME, FLAGS, BRANCH) "
                            "VALUES (?, ?, ?, ?, ?, ?)");

                        for (int j = 0; (j < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
                            const XSYMBOL &symbol = pState->listSymbols.at(j);

                            QString sName;

                            if (symbol.nStringIndex != (quint16)-1) {
                                sName = pState->listStrings.at(symbol.nStringIndex);
                            }

                            query.bindValue(0, listKeys.at(i));
                            query.bindValue(1, XBinary::segmentRelOffsetToAddress(&(pState->memoryMap), symbol.nRegionIndex, symbol.nRelOffset));
                            query.bindValue(2, symbol.nSize);
                            query.bindValue(3, sName);
                            query.bindValue(4, symbol.nFlags);
                            query.bindValue(5, symbol.nBranch);

                            bResult = querySQL(&query, true);
                        }
                    }

                    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
                        querySQL(&query, QString("DELETE FROM REFINFO WHERE FILETYPE = %1").arg(listKeys.at(i)), true);

                        qint32 nNumberOfRecords = pState->listRefs.count();

                        query.prepare(
                            "INSERT OR REPLACE INTO REFINFO (FILETYPE, ADDRESS, REFADDRESS, SIZE, FLAGS, BRANCH) "
                            "VALUES (?, ?, ?, ?, ?, ?)");

                        for (int j = 0; (j < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
                            const XREFINFO &refInfo = pState->listRefs.at(j);

                            query.bindValue(0, listKeys.at(i));
                            query.bindValue(1, XBinary::segmentRelOffsetToAddress(&(pState->memoryMap), refInfo.nRegionIndex, refInfo.nRelOffset));
                            query.bindValue(2, XBinary::segmentRelOffsetToAddress(&(pState->memoryMap), refInfo.nRegionIndexRef, refInfo.nRelOffsetRef));
                            query.bindValue(3, refInfo.nSize);
                            query.bindValue(4, refInfo.nFlags);
                            query.bindValue(5, refInfo.nBranch);

                            bResult = querySQL(&query, true);
                        }
                    }

                    if (XBinary::isPdStructNotCanceled(pPdStruct)) {
                        querySQL(&query, QString("DELETE FROM RECORDS WHERE FILETYPE = %1").arg(listKeys.at(i)), true);

                        qint32 nNumberOfRecords = pState->listRecords.count();

                        query.prepare(
                            "INSERT OR REPLACE INTO RECORDS (FILETYPE, ADDRESS, SIZE, FLAGS, BRANCH) "
                            "VALUES (?, ?, ?, ?, ?)");

                        for (int j = 0; (j < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
                            const XRECORD &record = pState->listRecords.at(j);

                            query.bindValue(0, listKeys.at(i));
                            query.bindValue(1, XBinary::segmentRelOffsetToAddress(&(pState->memoryMap), record.nRegionIndex, record.nRelOffset));
                            query.bindValue(2, record.nSize);
                            query.bindValue(3, record.nFlags);
                            query.bindValue(4, record.nBranch);

                            bResult = querySQL(&query, true);
                        }
                    }
                }
            }
        }

        dataBase.commit();
        // TODO

        dataBase.close();
    }

    dataBase = QSqlDatabase();
    QSqlDatabase::removeDatabase("local_db");
#else
    Q_UNUSED(sDBFileName)
    Q_UNUSED(pPdStruct)
#endif
    return bResult;
}
#ifdef USE_XPROCESS
QString XInfoDB::threadStatusToString(THREAD_STATUS threadStatus)
{
    QString sResult = tr("Unknown");

    if (threadStatus == THREAD_STATUS_PAUSED) {
        sResult = tr("Paused");
    } else if (threadStatus == THREAD_STATUS_RUNNING) {
        sResult = tr("Running");
    }

    return sResult;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::querySQL(QSqlQuery *pSqlQuery, const QString &sSQL, bool bWrite)
{
    //     #ifdef QT_DEBUG
    //         QElapsedTimer timer;
    //         timer.start();
    //     #endif
    bool bResult = false;

    bResult = pSqlQuery->exec(sSQL);

    // qDebug("%s", pSqlQuery->lastQuery().toLatin1().data());

#ifdef QT_DEBUG
    if ((pSqlQuery->lastError().text() != " ") && (pSqlQuery->lastError().text() != "")) {
        qDebug("%s", pSqlQuery->lastQuery().toLatin1().data());
        qDebug("%s", pSqlQuery->lastError().text().toLatin1().data());
    }
#endif
    //     #ifdef QT_DEBUG
    //         qDebug("%s", pSqlQuery->lastQuery().toLatin1().data());
    //         qDebug("%lld msec", timer.elapsed());
    //     #endif

    return bResult;
}
#endif
#ifdef QT_SQL_LIB
bool XInfoDB::querySQL(QSqlQuery *pSqlQuery, bool bWrite)
{
    //     #ifdef QT_DEBUG
    //         QElapsedTimer timer;
    //         timer.start();
    //     #endif
    bool bResult = false;
    bResult = pSqlQuery->exec();

    if (bResult && bWrite) {
        setDatabaseChanged(true);
    }

#ifdef QT_DEBUG
    if ((pSqlQuery->lastError().text() != " ") && (pSqlQuery->lastError().text() != "")) {
        qDebug("%s", pSqlQuery->lastQuery().toLatin1().data());
        qDebug("%s", pSqlQuery->executedQuery().toLatin1().data());
        qDebug("%s", pSqlQuery->lastError().text().toLatin1().data());
    }
#endif
    //     #ifdef QT_DEBUG
    //         qDebug("%s", pSqlQuery->lastQuery().toLatin1().data());
    //         qDebug("%lld msec", timer.elapsed());
    //     #endif

    return bResult;
}
#endif
#ifdef QT_SQL_LIB
QString XInfoDB::convertStringSQLTableName(const QString &sSQL)
{
    QString sResult;

    sResult = sSQL;
    sResult = sResult.replace("-", "_");
    sResult = sResult.replace("'", "''");

    return sResult;
}
#endif
#ifdef QT_SQL_LIB
QString XInfoDB::convertStringSQLValue(const QString &sSQL)
{
    QString sResult;

    sResult = sSQL;
    sResult = sResult.replace("'", "''");

    return sResult;
}
#endif

void XInfoDB::testFunction()
{
#ifdef USE_XPROCESS
#ifdef Q_OS_LINUX
    user_regs_struct regs = {};

    errno = 0;

    long int nRet = ptrace(PTRACE_GETREGS, g_statusCurrent.nThreadId, nullptr, &regs);

    qDebug("ptrace failed: %s", strerror(errno));

    if (nRet != -1) {
        qDebug("TODO");
    } else {
        qDebug("PTRACE_GETREGS error");
    }
#endif
#endif
}

void XInfoDB::setDebuggerState(bool bState)
{
    m_bIsDebugger = bState;
    initDB();
}

bool XInfoDB::isDebugger()
{
    return m_bIsDebugger;
}

QString XInfoDB::convertOpcodeString(XDisasmAbstract::DISASM_RESULT disasmResult, const XInfoDB::RI_TYPE &riType, const XDisasmAbstract::DISASM_OPTIONS &disasmOptions)
{
    QString sResult = disasmResult.sOperands;

    if (disasmResult.relType) {
        sResult = _convertOpcodeString(sResult, disasmResult.nXrefToRelative, riType, disasmOptions);
    }

    if (disasmResult.memType) {
        sResult = _convertOpcodeString(sResult, disasmResult.nXrefToMemory, riType, disasmOptions);
    }

    return sResult;
}

QString XInfoDB::_convertOpcodeString(const QString &sString, XADDR nAddress, const RI_TYPE &riType, const XDisasmAbstract::DISASM_OPTIONS &disasmOptions)
{
    Q_UNUSED(nAddress)
    Q_UNUSED(riType)
    Q_UNUSED(disasmOptions)

    QString sResult = sString;

    // QString sReplace = XInfoDB::recordInfoToString(getRecordInfoCache(nAddress), riType);
    // QString sOrigin;
    // // = g_disasmCore.getNumberString(nAddress);

    // if (disasmOptions.bIsUppercase) {
    //     sOrigin = sOrigin.toUpper();
    // }

    // if (sReplace != "") {
    //     sResult = sResult.replace(sOrigin, sReplace);
    // }

    return sResult;
}

void XInfoDB::setDatabaseChanged(bool bState)
{
    m_bIsDatabaseChanged = bState;
}

bool XInfoDB::isDatabaseChanged()
{
    return m_bIsDatabaseChanged;
}

bool XInfoDB::isAnalyzed(XBinary::FT fileType)
{
    bool bResult = false;

    if (isStatePresent(fileType)) {
        bResult = m_mapProfiles.value(fileType)->bIsAnalyzed;
    }

    return bResult;
}

bool XInfoDB::isStatePresent(XBinary::FT fileType)
{
    return m_mapProfiles.contains(fileType);
}

XInfoDB::STATE *XInfoDB::getState(XBinary::FT fileType)
{
    return m_mapProfiles.value(fileType);
}

void XInfoDB::readDataSlot(quint64 nOffset, char *pData, qint64 nSize)
{
    // #ifdef QT_DEBUG
    //     qDebug("%llx", nOffset);
    // #endif
    replaceMemory(nOffset, pData, nSize);
}

void XInfoDB::writeDataSlot(quint64 nOffset, char *pData, qint64 nSize)
{
#ifdef QT_DEBUG
    qDebug("%llx", nOffset);
#endif
    replaceMemory(nOffset, pData, nSize);
}

void XInfoDB::replaceMemory(quint64 nOffset, char *pData, qint64 nSize)
{
#ifndef USE_XPROCESS
    Q_UNUSED(nOffset)
    Q_UNUSED(pData)
    Q_UNUSED(nSize)
#endif
#ifdef USE_XPROCESS
    qint32 nNumberOfBreakPoints = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfBreakPoints; i++) {
        if (g_listBreakpoints.at(i).nDataSize && XBinary::_isAddressCrossed(nOffset, nSize, g_listBreakpoints.at(i).nAddress, g_listBreakpoints.at(i).nDataSize)) {
#ifdef QT_DEBUG
            qDebug("Breakpoint replace: %llX", g_listBreakpoints.at(i).nAddress);
            qDebug("nOffset: %llX", nOffset);
#endif
            char *pSource = nullptr;
            char *pDest = nullptr;
            qint64 nDataSize = 0;

            if (g_listBreakpoints.at(i).nAddress >= nOffset) {
                pSource = (char *)g_listBreakpoints.at(i).origData;
                pDest = pData + (g_listBreakpoints.at(i).nAddress - nOffset);
                nDataSize = qMin((quint64)g_listBreakpoints.at(i).nDataSize, nOffset + nSize - g_listBreakpoints.at(i).nAddress);
            } else if (nOffset > g_listBreakpoints.at(i).nAddress) {
                pSource = (char *)g_listBreakpoints.at(i).origData + (nOffset - g_listBreakpoints.at(i).nAddress);
                pDest = pData;
                nDataSize = qMin((quint64)g_listBreakpoints.at(i).nDataSize, nOffset - g_listBreakpoints.at(i).nAddress);
            }

            if (pSource && pDest && nDataSize) {
                XBinary::_copyMemory(pDest, pSource, nDataSize);
            }
        }
    }
#endif
}
#ifdef USE_XPROCESS
XBinary::XVARIANT XInfoDB::_getRegCache(QList<REG_RECORD> *pListRegs, XREG reg)
{
    // TODO AX AL AH
    XBinary::XVARIANT result = {};

    XREG _reg = reg;
#ifdef Q_PROCESSOR_X86
    if ((reg == XREG_FLAGS_CF) || (reg == XREG_FLAGS_PF) || (reg == XREG_FLAGS_AF) || (reg == XREG_FLAGS_ZF) || (reg == XREG_FLAGS_SF) || (reg == XREG_FLAGS_TF) ||
        (reg == XREG_FLAGS_IF) || (reg == XREG_FLAGS_DF) || (reg == XREG_FLAGS_OF)) {
#ifdef Q_PROCESSOR_X86_32
        _reg = XREG_EFLAGS;
#endif
#ifdef Q_PROCESSOR_X86_64
        _reg = XREG_RFLAGS;
#endif
    }
#endif
#ifdef Q_PROCESSOR_X86_32
    // TODO
#endif
#ifdef Q_PROCESSOR_X86_64
    // TODO
#endif

    qint32 nNumberOfRecords = pListRegs->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListRegs->at(i).reg == _reg) {
            result = pListRegs->at(i).value;
            break;
        }
    }

    if (result.varType != XBinary::VT_UNKNOWN) {
#ifdef Q_PROCESSOR_X86
        if ((reg == XREG_FLAGS_CF) || (reg == XREG_FLAGS_PF) || (reg == XREG_FLAGS_AF) || (reg == XREG_FLAGS_ZF) || (reg == XREG_FLAGS_SF) || (reg == XREG_FLAGS_TF) ||
            (reg == XREG_FLAGS_IF) || (reg == XREG_FLAGS_DF) || (reg == XREG_FLAGS_OF)) {
            result = getFlagFromReg(result, reg);
        }
#endif
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::_setRegCache(QList<REG_RECORD> *pListRegs, XREG reg, XBinary::XVARIANT variant)
{
    // mb TODO handle flags
    bool bFound = false;
    qint32 nNumberOfRecords = pListRegs->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListRegs->at(i).reg == reg) {
            REG_RECORD record = {};
            record.reg = reg;
            record.value = variant;

            pListRegs->replace(i, record);
            bFound = true;
            break;
        }
    }

    if (!bFound) {
        _addCurrentRegRecord(reg, variant);
    }
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::_addCurrentRegRecord(XREG reg, XBinary::XVARIANT value)
{
    REG_RECORD record = {};
    record.reg = reg;
    record.value = value;

    g_statusCurrent.listRegs.append(record);
}
#endif
#ifdef USE_XPROCESS
XBinary::XVARIANT XInfoDB::getFlagFromReg(XBinary::XVARIANT variant, XREG reg)
{
    XBinary::XVARIANT result = variant;

    if (variant.varType == XBinary::VT_DWORD) {
#ifdef Q_PROCESSOR_X86_32
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.toULongLong(), RFLAGS_BIT_OF));
#endif
    } else if (variant.varType == XBinary::VT_QWORD) {
#ifdef Q_PROCESSOR_X86_64
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.toULongLong(), RFLAGS_BIT_OF));
#endif
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XBinary::XVARIANT XInfoDB::setFlagToReg(XBinary::XVARIANT variant, XREG reg, bool bValue)
{
    // TODO setBit in Xbinary
    XBinary::XVARIANT result = variant;

    if (variant.varType == XBinary::VT_DWORD) {
#ifdef Q_PROCESSOR_X86_32
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.toULongLong(), bValue, RFLAGS_BIT_OF));
#endif
    } else if (variant.varType == XBinary::VT_QWORD) {
#ifdef Q_PROCESSOR_X86_64
        // TODO Check mb reg32
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.toULongLong(), bValue, RFLAGS_BIT_OF));
#endif
    }

    return result;
}
#endif
