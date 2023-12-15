/* Copyright (c) 2020-2023 hors<horsicq@gmail.com>
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

bool _symbolSort(const XInfoDB::SYMBOL &v1, const XInfoDB::SYMBOL &v2)
{
    bool bResult = false;

    if (v1.nModule != v2.nModule) {
        bResult = (v1.nModule < v2.nModule);
    } else {
        bResult = (v1.nAddress < v2.nAddress);
    }

    return bResult;
}

XInfoDB::XInfoDB(QObject *pParent) : QObject(pParent)
{
    g_mode = MODE_UNKNOWN;
#ifdef USE_XPROCESS
    g_processInfo = {};

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
    g_pDevice = nullptr;
    g_handle = 0;
    g_fileType = XBinary::FT_UNKNOWN;
    g_nMainModuleAddress = 0;
    g_nMainModuleSize = 0;
    g_bIsDebugger = false;
    g_disasmMode = XBinary::DM_UNKNOWN;
    g_pMutexSQL = new QMutex;
    g_pMutexThread = new QMutex;
    g_bIsDatabaseChanged = false;
}

XInfoDB::~XInfoDB()
{
    XCapstone::closeHandle(&g_handle);
#ifdef QT_SQL_LIB
#ifdef QT_DEBUG
    qDebug("XInfoDB::~XInfoDB()");
#endif
    if (g_dataBase.isOpen()) {
        g_dataBase.close();
        g_dataBase = QSqlDatabase();
        QSqlDatabase::removeDatabase("memory_db");
    }
#endif
    delete g_pMutexSQL;
    delete g_pMutexThread;
}

void XInfoDB::setData(QIODevice *pDevice, XBinary::FT fileType, XBinary::DM disasmMode)
{
    g_pDevice = pDevice;
    g_mode = MODE_DEVICE;
    g_fileType = fileType;

    g_binary.setDevice(pDevice);  // TODO read/write signals

    if (fileType == XBinary::FT_UNKNOWN) {
        g_fileType = XBinary::getPrefFileType(g_pDevice);
    }

    g_disasmMode = disasmMode;

    if (disasmMode == XBinary::DM_UNKNOWN) {
        g_disasmMode = XBinary::getDisasmMode(XFormats::getOsInfo(g_fileType, g_pDevice));
    }

    XCapstone::closeHandle(&g_handle);
    XCapstone::openHandle(g_disasmMode, &g_handle, true);

    g_MainModuleMemoryMap = XFormats::getMemoryMap(g_fileType, g_pDevice);

    g_nMainModuleAddress = g_MainModuleMemoryMap.nModuleAddress;
    g_nMainModuleSize = g_MainModuleMemoryMap.nImageSize;

    g_sMainModuleName = XBinary::getDeviceFileBaseName(g_pDevice);

    _createTableNames();

    initDB();

    initHexDb();
    initDisasmDb();  // TODO Check

    // reloadView();
}

QIODevice *XInfoDB::getDevice()
{
    return g_pDevice;
}

void XInfoDB::initDB()
{
#ifdef QT_SQL_LIB
    if (!g_dataBase.isOpen()) {
        g_dataBase = QSqlDatabase::addDatabase("QSQLITE", "memory_db");

        //        g_dataBase.setDatabaseName(":memory:");
#ifdef Q_OS_WIN
        g_dataBase.setDatabaseName(":memory:");
        //        g_dataBase.setDatabaseName("C:\\tmp_build\\local_dbXS.db");
#else
        g_dataBase.setDatabaseName(":memory:");
#endif
        // #ifdef Q_OS_LINUX
        //     g_dataBase.setDatabaseName("/home/hors/local_db.db");
        // #endif
        //  #ifndef QT_DEBUG
        //      g_dataBase.setDatabaseName(":memory:");
        //  #else
        //  #ifdef Q_OS_WIN
        //      g_dataBase.setDatabaseName("C:\\tmp_build\\local_dbXS.db");
        ////    g_dataBase.setDatabaseName(":memory:");
        // #else
        //     g_dataBase.setDatabaseName(":memory:");
        // #endif
        ////    g_dataBase.setDatabaseName(":memory:");
        // #endif

        if (g_dataBase.open()) {
            g_dataBase.exec("PRAGMA synchronous = OFF");
            g_dataBase.exec("PRAGMA journal_mode = MEMORY");

            // setAnalyzed(isSymbolsPresent());
        } else {
#ifdef QT_DEBUG
            qDebug("Cannot open sqlite database");
#endif
        }
    }
#endif
}

XBinary::FT XInfoDB::getFileType()
{
    return g_fileType;
}

XBinary::DM XInfoDB::getDisasmMode()
{
    return g_disasmMode;
}

void XInfoDB::reload(bool bReloadData)
{
    // TODO Check
    emit reloadSignal(bReloadData);
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

void XInfoDB::_createTableNames()
{
#ifdef QT_SQL_LIB
    s_sql_tableName[DBTABLE_SYMBOLS] = convertStringSQLTableName(QString("%1_SYMBOLS").arg(XBinary::disasmIdToString(g_disasmMode)));
    s_sql_tableName[DBTABLE_SHOWRECORDS] = convertStringSQLTableName(QString("%1_SHOWRECORDS").arg(XBinary::disasmIdToString(g_disasmMode)));
    s_sql_tableName[DBTABLE_RELATIVS] = convertStringSQLTableName(QString("%1_RELRECORDS").arg(XBinary::disasmIdToString(g_disasmMode)));
    s_sql_tableName[DBTABLE_IMPORT] = convertStringSQLTableName(QString("%1_IMPORT").arg(XBinary::disasmIdToString(g_disasmMode)));
    s_sql_tableName[DBTABLE_EXPORT] = convertStringSQLTableName(QString("%1_EXPORT").arg(XBinary::disasmIdToString(g_disasmMode)));
    s_sql_tableName[DBTABLE_TLS] = convertStringSQLTableName(QString("%1_TLS").arg(XBinary::disasmIdToString(g_disasmMode)));
    s_sql_tableName[DBTABLE_FUNCTIONS] = convertStringSQLTableName(QString("%1_FUNCTIONS").arg(XBinary::disasmIdToString(g_disasmMode)));
    s_sql_tableName[DBTABLE_BOOKMARKS] = convertStringSQLTableName(QString("BOOKMARKS"));
    s_sql_tableName[DBTABLE_COMMENTS] = convertStringSQLTableName(QString("COMMENTS"));
#endif
}

#ifdef USE_XPROCESS
quint32 XInfoDB::read_uint32(XADDR nAddress, bool bIsBigEndian)
{
    quint32 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::read_uint32(g_processInfo.hProcess, nAddress, bIsBigEndian);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::read_uint32(g_processInfo.hProcessMemoryIO, nAddress, bIsBigEndian);
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
quint64 XInfoDB::read_uint64(XADDR nAddress, bool bIsBigEndian)
{
    quint64 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::read_uint64(g_processInfo.hProcess, nAddress, bIsBigEndian);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::read_uint64(g_processInfo.hProcessMemoryIO, nAddress, bIsBigEndian);
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
qint64 XInfoDB::read_array(XADDR nAddress, char *pData, quint64 nSize)
{
    qint64 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::read_array(g_processInfo.hProcess, nAddress, pData, nSize);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::read_array(g_processInfo.hProcessMemoryIO, nAddress, pData, nSize);
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
qint64 XInfoDB::write_array(XADDR nAddress, char *pData, quint64 nSize)
{
    qint64 nResult = 0;
#ifdef Q_OS_WIN
    nResult = XProcess::write_array(g_processInfo.hProcess, nAddress, pData, nSize);
#endif
#ifdef Q_OS_LINUX
    nResult = XProcess::write_array(g_processInfo.hProcessMemoryIO, nAddress, pData, nSize);
#endif
    return nResult;
}
#endif
#ifdef USE_XPROCESS
QByteArray XInfoDB::read_array(XADDR nAddress, quint64 nSize)
{
    QByteArray baResult;
#ifdef Q_OS_WIN
    baResult = XProcess::read_array(g_processInfo.hProcess, nAddress, nSize);
#endif
#ifdef Q_OS_LINUX
    baResult = XProcess::read_array(g_processInfo.hProcessMemoryIO, nAddress, nSize);
#endif
    return baResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::read_ansiString(XADDR nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef Q_OS_WIN
    sResult = XProcess::read_ansiString(g_processInfo.hProcess, nAddress, nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult = XProcess::read_ansiString(g_processInfo.hProcessMemoryIO, nAddress, nMaxSize);
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::read_unicodeString(XADDR nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef Q_OS_WIN
    sResult = XProcess::read_unicodeString(g_processInfo.hProcess, nAddress, nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult = XProcess::read_unicodeString(g_processInfo.hProcessMemoryIO, nAddress, nMaxSize);
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
QString XInfoDB::read_utf8String(XADDR nAddress, quint64 nMaxSize)
{
    QString sResult;
#ifdef Q_OS_WIN
    sResult = XProcess::read_utf8String(g_processInfo.hProcess, nAddress, nMaxSize);
#endif
#ifdef Q_OS_LINUX
    sResult = XProcess::read_utf8String(g_processInfo.hProcessMemoryIO, nAddress, nMaxSize);
#endif
    return sResult;
}
#endif
#ifdef USE_XPROCESS
XCapstone::DISASM_RESULT XInfoDB::disasm(XADDR nAddress)
{
    QByteArray baArray = read_array(nAddress, 16);

    return XCapstone::disasm_ex(g_handle, getDisasmMode(), XBinary::SYNTAX_DEFAULT, baArray.data(), baArray.size(), nAddress);
}
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
#ifndef USE_XPROCESS
quint32 XInfoDB::read_uint32(qint64 nOffset, bool bIsBigEndian)
{
    return g_binary.read_uint32(nOffset, bIsBigEndian);
}
#endif
#ifndef USE_XPROCESS
quint64 XInfoDB::read_uint64(qint64 nOffset, bool bIsBigEndian)
{
    return g_binary.read_uint64(nOffset, bIsBigEndian);
}
#endif
#ifndef USE_XPROCESS
qint64 XInfoDB::read_array(qint64 nOffset, char *pData, quint64 nSize)
{
    return g_binary.read_array(nOffset, pData, nSize);
}
#endif
#ifndef USE_XPROCESS
qint64 XInfoDB::write_array(qint64 nOffset, char *pData, quint64 nSize)
{
    return g_binary.write_array(nOffset, pData, nSize);
}
#endif
#ifndef USE_XPROCESS
QByteArray XInfoDB::read_array(qint64 nOffset, quint64 nSize)
{
    return g_binary.read_array(nOffset, nSize);
}
#endif
#ifndef USE_XPROCESS
QString XInfoDB::read_ansiString(qint64 nOffset, quint64 nMaxSize)
{
    return g_binary.read_ansiString(nOffset, nMaxSize);
}
#endif
#ifndef USE_XPROCESS
QString XInfoDB::read_unicodeString(qint64 nOffset, quint64 nMaxSize)
{
    return g_binary.read_unicodeString(nOffset, nMaxSize);
}
#endif
#ifndef USE_XPROCESS
QString XInfoDB::read_utf8String(qint64 nOffset, quint64 nMaxSize)
{
    return g_binary.read_utf8String(nOffset, nMaxSize);
}
#endif
QList<QString> XInfoDB::getStringsFromFile(const QString &sFileName)
{
    QList<QString> listResult;

    QFile inputFile(sFileName);

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        while (!in.atEnd()) {
            QString sLine = in.readLine();

            listResult.append(sLine);
        }
        inputFile.close();
    }

    return listResult;
}

XInfoDB::STRRECORD XInfoDB::handleStringDB(QList<QString> *pListStrings, const QString &sString, bool bIsMulti)
{
    STRRECORD result = {};

    qint32 nNumberOfRecords = pListStrings->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        QString sRecord = pListStrings->at(i);

        if (sRecord.contains("|")) {
            QString sValue = sRecord.section("|", 0, -3);

            if (sString == sValue) {
                QString sType = sRecord.section("|", -2, -2);
                QString _sString = sRecord.section("|", -1, -1);

                if (result.sDescription != "") {
                    result.sDescription += " | ";
                }

                if (sType != "") {
                    result.sDescription += QString("(%1) ").arg(XFormats::translateType(sType));
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
    }

    return result;
}

QList<QString> XInfoDB::loadStrDB(const QString &sPath, STRDB strDB)
{
    QList<QString> listResult;

    QString sStrDBFileName;

    if (strDB == STRDB_PESECTIONS) {
        sStrDBFileName = "PE.sections.txt";
    }

    if (sStrDBFileName != "") {
        listResult = getStringsFromFile(XBinary::convertPathName(sPath) + QDir::separator() + sStrDBFileName);
    }

    return listResult;
}
#ifdef USE_XPROCESS
void XInfoDB::setDefaultBreakpointType(BPT bpType)
{
    g_bpTypeDefault = bpType;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentThreadId(X_ID nThreadId)
{
    g_statusCurrent.nThreadId = nThreadId;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentThreadHandle(X_HANDLE hThread)
{
    g_statusCurrent.hThread = hThread;
}
#endif
#ifdef USE_XPROCESS
X_ID XInfoDB::getCurrentThreadId()
{
    return g_statusCurrent.nThreadId;
}
#endif
#ifdef USE_XPROCESS
X_HANDLE XInfoDB::getCurrentThreadHandle()
{
    return g_statusCurrent.hThread;
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
        bpType = g_bpTypeDefault;
    }

    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if ((g_listBreakpoints.at(i).nAddress == nAddress) && (g_listBreakpoints.at(i).bpType == bpType)) {
            result = g_listBreakpoints.at(i);

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
        bpType = g_bpTypeDefault;
    }

    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XInfoDB::BREAKPOINT breakPoint = g_listBreakpoints.at(i);

        if ((breakPoint.nAddress == (nExceptionAddress - breakPoint.nDataSize)) && (g_listBreakpoints.at(i).bpType == bpType)) {
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

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XInfoDB::BREAKPOINT breakPoint = g_listBreakpoints.at(i);

        if ((breakPoint.nThreadID == nThreadID) && (g_listBreakpoints.at(i).bpType == bpType)) {
            result = breakPoint;

            break;
        }
    }

    return result;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::BREAKPOINT XInfoDB::findBreakPointByUUID(QString sUUID)
{
    BREAKPOINT result = {};
    result.nAddress = -1;

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (g_listBreakpoints.at(i).sUUID == sUUID) {
            result = g_listBreakpoints.at(i);

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

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (XBinary::_isAddressCrossed(g_listBreakpoints.at(i).nAddress, g_listBreakpoints.at(i).nDataSize, nAddress, nSize)) {
            result = g_listBreakpoints.at(i);

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

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (g_listBreakpoints.at(i).nThreadID == nThreadID) {
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
    g_mapSharedObjectInfos.insert(pSharedObjectInfo->nImageBase, *pSharedObjectInfo);
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
    g_pMutexThread->lock();
    g_listThreadInfos.append(*pThreadInfo);
    g_pMutexThread->unlock();
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::removeThreadInfo(X_ID nThreadID)
{
    g_pMutexThread->lock();

    qint32 nNumberOfThread = g_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfThread; i++) {
        if (g_listThreadInfos.at(i).nThreadID == nThreadID) {
            g_listThreadInfos.removeAt(i);

            break;
        }
    }

    g_pMutexThread->unlock();
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::setThreadStatus(X_ID nThreadID, THREAD_STATUS status)
{
    g_pMutexThread->lock();

    bool bResult = false;
    qint32 nNumberOfThread = g_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfThread; i++) {
        if (g_listThreadInfos.at(i).nThreadID == nThreadID) {
            g_listThreadInfos[i].threadStatus = status;

            break;
        }
    }

    g_pMutexThread->unlock();

    return bResult;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::THREAD_STATUS XInfoDB::getThreadStatus(X_ID nThreadID)
{
    g_pMutexThread->lock();

    THREAD_STATUS result = THREAD_STATUS_UNKNOWN;
    qint32 nNumberOfThread = g_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfThread; i++) {
        if (g_listThreadInfos.at(i).nThreadID == nThreadID) {
            result = g_listThreadInfos[i].threadStatus;

            break;
        }
    }

    g_pMutexThread->unlock();

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

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    // TODO Check!
    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XInfoDB::BREAKPOINT breakPoint = g_listBreakpoints.at(i);

        if (breakPoint.vInfo.toString() == sFunctionName) {
            g_listBreakpoints.removeAt(i);
        }
    }

    if (g_mapFunctionHookInfos.contains(sFunctionName)) {
        g_mapFunctionHookInfos.remove(sFunctionName);

        bResult = true;
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
QMap<XADDR, XInfoDB::SHAREDOBJECT_INFO> *XInfoDB::getSharedObjectInfos()
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
QMap<QString, XInfoDB::FUNCTIONHOOK_INFO> *XInfoDB::getFunctionHookInfos()
{
    return &g_mapFunctionHookInfos;
}
#endif
#ifdef USE_XPROCESS
XInfoDB::SHAREDOBJECT_INFO XInfoDB::findSharedInfoByName(const QString &sName)
{
    XInfoDB::SHAREDOBJECT_INFO result = {};

    for (QMap<XADDR, XInfoDB::SHAREDOBJECT_INFO>::iterator it = g_mapSharedObjectInfos.begin(); it != g_mapSharedObjectInfos.end();) {
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

    for (QMap<XADDR, XInfoDB::SHAREDOBJECT_INFO>::iterator it = g_mapSharedObjectInfos.begin(); it != g_mapSharedObjectInfos.end();) {
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

    g_pMutexThread->lock();

    qint32 nNumberOfRecords = g_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (g_listThreadInfos.at(i).nThreadID == nThreadID) {
            result = g_listThreadInfos.at(i);

            break;
        }
    }

    g_pMutexThread->unlock();

    return result;
}
#endif
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
XInfoDB::THREAD_INFO XInfoDB::findThreadInfoByHandle(X_HANDLE hThread)
{
    XInfoDB::THREAD_INFO result = {};

    g_pMutexThread->lock();

    qint32 nNumberOfRecords = g_listThreadInfos.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (g_listThreadInfos.at(i).hThread == hThread) {
            result = g_listThreadInfos.at(i);

            break;
        }
    }

    g_pMutexThread->unlock();

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
    XADDR nResult = -1;

    QByteArray baData = read_array(nAddress, 15);

    XCapstone::OPCODE_ID opcodeID = XCapstone::getOpcodeID(g_handle, nAddress, baData.data(), baData.size());

    if (XCapstone::isCallOpcode(XBinary::getDisasmFamily(g_disasmMode), opcodeID.nOpcodeID)) {
        nResult = nAddress + opcodeID.nSize;
    }

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
    if (syscall(SYS_tgkill, g_processInfo.nProcessID, nThreadId, SIGSTOP) != -1) {
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

    g_pMutexThread->lock();

    qint32 nCount = g_listThreadInfos.count();

    // TODO Check if already suspended
    for (qint32 i = 0; i < nCount; i++) {
#ifdef Q_OS_WIN
        if (g_listThreadInfos.at(i).threadStatus == THREAD_STATUS_RUNNING) {
            if (suspendThread_Handle(g_listThreadInfos.at(i).hThread)) {
                g_listThreadInfos[i].threadStatus = THREAD_STATUS_PAUSED;
            }
        }
#endif
#ifdef Q_OS_LINUX
        if (syscall(SYS_tgkill, g_processInfo.nProcessID, g_listThreadInfos.at(i).nThreadID, SIGSTOP) != -1) {
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

    g_pMutexThread->unlock();

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::resumeAllThreads()
{
    bool bResult = false;

    g_pMutexThread->lock();

    qint32 nCount = g_listThreadInfos.count();

    // Resume all other threads
    for (qint32 i = 0; i < nCount; i++) {
#ifdef Q_OS_WIN
        if (g_listThreadInfos.at(i).threadStatus == THREAD_STATUS_PAUSED) {
            if (resumeThread_Handle(g_listThreadInfos.at(i).hThread)) {
                g_listThreadInfos[i].threadStatus = THREAD_STATUS_RUNNING;
            }
        }
#endif
#ifdef Q_OS_LINUX
        if (g_listThreadInfos.at(i).threadStatus == THREAD_STATUS_PAUSED) {
            if (ptrace(PTRACE_CONT, g_listThreadInfos.at(i).nThreadID, 0, 0) != -1) {
                g_listThreadInfos[i].threadStatus = THREAD_STATUS_RUNNING;
            }
        }
#endif

        bResult = true;
    }

    g_pMutexThread->unlock();

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
    g_processInfo = processInfo;
    g_mode = MODE_PROCESS;

    g_nMainModuleAddress = processInfo.nImageBase;
    g_nMainModuleSize = processInfo.nImageSize;
    g_sMainModuleName = g_processInfo.sBaseFileName;
    // g_MainModuleMemoryMap=XFormats::getMemoryMap(XBinary::FT_REGION,0,true,)
    // // TODO getRegionMemoryMap
#ifdef USE_XPROCESS
#ifdef Q_PROCESSOR_X86_32
    g_disasmMode = XBinary::DM_X86_32;
#endif
#ifdef Q_PROCESSOR_X86_64
    g_disasmMode = XBinary::DM_X86_64;
#endif
#endif

    XCapstone::closeHandle(&g_handle);
    XCapstone::openHandle(g_disasmMode, &g_handle, true);

    _createTableNames();

    initDB();

    initHexDb();
    initDisasmDb();  // TODO Check
}
#endif
#ifdef USE_XPROCESS
XInfoDB::PROCESS_INFO *XInfoDB::getProcessInfo()
{
    return &g_processInfo;
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateRegsById(X_ID nThreadId, const XREG_OPTIONS &regOptions)
{
    // TODO HASH !!!
    g_statusCurrent.listRegsPrev = g_statusCurrent.listRegs;  // TODO save nThreadID

    g_statusCurrent.listRegs.clear();
    g_statusCurrent.nThreadId = nThreadId;

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
    g_statusCurrent.hThread = hThread;

#ifdef Q_OS_WIN
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_ALL;  // All registers TODO Check regOptions
                                         // |CONTEXT_FLOATING_POINT|CONTEXT_EXTENDED_REGISTERS

    if (GetThreadContext(hThread, &context)) {
        quint32 nRegistersHash = XBinary::_getCRC32((char *)&context, sizeof(context), 0, XBinary::_getCRC32Table_EDB88320());

        if (g_statusCurrent.nRegistersHash != nRegistersHash) {
            g_statusCurrent.nRegistersHash = nRegistersHash;

            g_statusCurrent.listRegsPrev = g_statusCurrent.listRegs;  // TODO save nThreadID

            g_statusCurrent.listRegs.clear();

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
    quint32 nMemoryRegionsHash = XProcess::getMemoryRegionsListHash_Handle(g_processInfo.hProcess);
#endif
#ifdef Q_OS_LINUX
    quint32 nMemoryRegionsHash = XProcess::getMemoryRegionsListHash_Id(g_processInfo.nProcessID);
#endif
    if (g_statusCurrent.nMemoryRegionsHash != nMemoryRegionsHash) {
        g_statusCurrent.nMemoryRegionsHash = nMemoryRegionsHash;
#ifdef Q_OS_WIN
        g_statusCurrent.listMemoryRegions = XProcess::getMemoryRegionsList_Handle(g_processInfo.hProcess, 0, 0xFFFFFFFFFFFFFFFF);
#endif
#ifdef Q_OS_LINUX
        g_statusCurrent.listMemoryRegions = XProcess::getMemoryRegionsList_Handle(g_processInfo.hProcessMemoryQuery, 0, 0xFFFFFFFFFFFFFFFF);
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
    quint32 nModulesHash = XProcess::getModulesListHash(g_processInfo.nProcessID);

    if (g_statusCurrent.nModulesHash != nModulesHash) {
        g_statusCurrent.nModulesHash = nModulesHash;
        g_statusCurrent.listModules = XProcess::getModulesList(g_processInfo.nProcessID);

        emit modulesListChanged();
    }
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::updateThreadsList()
{
    // mb TODO function for compare 2 lists
    //    g_statusPrev.listModules = g_statusCurrent.listModules;
    quint32 nThreadsHash = XProcess::getThreadsListHash(g_processInfo.nProcessID);

    if (g_statusCurrent.nThreadsHash != nThreadsHash) {
        g_statusCurrent.nThreadsHash = nThreadsHash;
        g_statusCurrent.listThreads = XProcess::getThreadsList(g_processInfo.nProcessID);

        emit threadsListChanged();
    }
}
#endif
#ifdef USE_XPROCESS
XBinary::XVARIANT XInfoDB::getCurrentRegCache(XREG reg)
{
    return _getRegCache(&(g_statusCurrent.listRegs), reg);
}
#endif
#ifdef USE_XPROCESS
void XInfoDB::setCurrentRegCache(XREG reg, XBinary::XVARIANT variant)
{
    _setRegCache(&(g_statusCurrent.listRegs), reg, variant);
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
        if (reg == XREG_EAX) context.Eax = variant.var.v_uint32;
        else if (reg == XREG_EBX) context.Ebx = variant.var.v_uint32;
        else if (reg == XREG_ECX) context.Ecx = variant.var.v_uint32;
        else if (reg == XREG_EDX) context.Edx = variant.var.v_uint32;
        else if (reg == XREG_EBP) context.Ebp = variant.var.v_uint32;
        else if (reg == XREG_ESP) context.Esp = variant.var.v_uint32;
        else if (reg == XREG_ESI) context.Esi = variant.var.v_uint32;
        else if (reg == XREG_EDI) context.Edi = variant.var.v_uint32;
        else if (reg == XREG_EFLAGS) context.EFlags = variant.var.v_uint32;
        else if (reg == XREG_EIP) context.Eip = variant.var.v_uint32;
        else if (reg == XREG_DR0) context.Dr0 = variant.var.v_uint32;
        else if (reg == XREG_DR1) context.Dr1 = variant.var.v_uint32;
        else if (reg == XREG_DR2) context.Dr2 = variant.var.v_uint32;
        else if (reg == XREG_DR3) context.Dr3 = variant.var.v_uint32;
        else if (reg == XREG_DR6) context.Dr6 = variant.var.v_uint32;
        else if (reg == XREG_DR7) context.Dr7 = variant.var.v_uint32;
        else if ((reg == XInfoDB::XREG_FLAGS_CF) || (reg == XInfoDB::XREG_FLAGS_PF) || (reg == XInfoDB::XREG_FLAGS_AF) || (reg == XInfoDB::XREG_FLAGS_ZF) ||
                 (reg == XInfoDB::XREG_FLAGS_SF) || (reg == XInfoDB::XREG_FLAGS_TF) || (reg == XInfoDB::XREG_FLAGS_IF) || (reg == XInfoDB::XREG_FLAGS_DF) ||
                 (reg == XInfoDB::XREG_FLAGS_OF)) {
            context.EFlags = setFlagToReg(XBinary::getXVariant((quint64)context.EFlags), reg, variant.var.v_bool).var.v_uint32;
        } else bUnknownRegister = true;
#endif
#ifdef Q_PROCESSOR_X86_64
        if (reg == XREG_RAX) context.Rax = variant.var.v_uint64;
        else if (reg == XREG_RBX) context.Rbx = variant.var.v_uint64;
        else if (reg == XREG_RCX) context.Rcx = variant.var.v_uint64;
        else if (reg == XREG_RDX) context.Rdx = variant.var.v_uint64;
        else if (reg == XREG_RBP) context.Rbp = variant.var.v_uint64;
        else if (reg == XREG_RSP) context.Rsp = variant.var.v_uint64;
        else if (reg == XREG_RSI) context.Rsi = variant.var.v_uint64;
        else if (reg == XREG_RDI) context.Rdi = variant.var.v_uint64;
        else if (reg == XREG_R8) context.R8 = variant.var.v_uint64;
        else if (reg == XREG_R9) context.R9 = variant.var.v_uint64;
        else if (reg == XREG_R10) context.R10 = variant.var.v_uint64;
        else if (reg == XREG_R11) context.R11 = variant.var.v_uint64;
        else if (reg == XREG_R12) context.R12 = variant.var.v_uint64;
        else if (reg == XREG_R13) context.R13 = variant.var.v_uint64;
        else if (reg == XREG_R14) context.R14 = variant.var.v_uint64;
        else if (reg == XREG_R15) context.R15 = variant.var.v_uint64;
        else if (reg == XREG_RFLAGS) context.EFlags = variant.var.v_uint64;
        else if (reg == XREG_RIP) context.Rip = variant.var.v_uint64;
        else if (reg == XREG_DR0) context.Dr0 = variant.var.v_uint64;
        else if (reg == XREG_DR1) context.Dr1 = variant.var.v_uint64;
        else if (reg == XREG_DR2) context.Dr2 = variant.var.v_uint64;
        else if (reg == XREG_DR3) context.Dr3 = variant.var.v_uint64;
        else if (reg == XREG_DR6) context.Dr6 = variant.var.v_uint64;
        else if (reg == XREG_DR7) context.Dr7 = variant.var.v_uint64;
        else if ((reg == XInfoDB::XREG_FLAGS_CF) || (reg == XInfoDB::XREG_FLAGS_PF) || (reg == XInfoDB::XREG_FLAGS_AF) || (reg == XInfoDB::XREG_FLAGS_ZF) ||
                 (reg == XInfoDB::XREG_FLAGS_SF) || (reg == XInfoDB::XREG_FLAGS_TF) || (reg == XInfoDB::XREG_FLAGS_IF) || (reg == XInfoDB::XREG_FLAGS_DF) ||
                 (reg == XInfoDB::XREG_FLAGS_OF)) {
            context.EFlags = setFlagToReg(XBinary::getXVariant((quint64)context.EFlags), reg, variant.var.v_bool).var.v_uint64;
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
            if (reg == XREG_EAX) regs.eax = variant.var.v_uint32;
            else if (reg == XREG_EBX) regs.ebx = variant.var.v_uint32;
            else if (reg == XREG_ECX) regs.ecx = variant.var.v_uint32;
            else if (reg == XREG_EDX) regs.edx = variant.var.v_uint32;
            else if (reg == XREG_EBP) regs.ebp = variant.var.v_uint32;
            else if (reg == XREG_ESP) regs.esp = variant.var.v_uint32;
            else if (reg == XREG_ESI) regs.esi = variant.var.v_uint32;
            else if (reg == XREG_EDI) regs.edi = variant.var.v_uint32;
            else if (reg == XREG_EIP) regs.eip = variant.var.v_uint32;
            else if (reg == XREG_EFLAGS) regs.eflags = variant.var.v_uint32;
#endif
#ifdef Q_PROCESSOR_X86_64
            if (reg == XREG_RAX) regs.rax = variant.var.v_uint64;
            else if (reg == XREG_RBX) regs.rbx = variant.var.v_uint64;
            else if (reg == XREG_RCX) regs.rcx = variant.var.v_uint64;
            else if (reg == XREG_RDX) regs.rdx = variant.var.v_uint64;
            else if (reg == XREG_RBP) regs.rbp = variant.var.v_uint64;
            else if (reg == XREG_RSP) regs.rsp = variant.var.v_uint64;
            else if (reg == XREG_RSI) regs.rsi = variant.var.v_uint64;
            else if (reg == XREG_RDI) regs.rdi = variant.var.v_uint64;
            else if (reg == XREG_R8) regs.r8 = variant.var.v_uint64;
            else if (reg == XREG_R9) regs.r9 = variant.var.v_uint64;
            else if (reg == XREG_R10) regs.r10 = variant.var.v_uint64;
            else if (reg == XREG_R11) regs.r11 = variant.var.v_uint64;
            else if (reg == XREG_R12) regs.r12 = variant.var.v_uint64;
            else if (reg == XREG_R13) regs.r13 = variant.var.v_uint64;
            else if (reg == XREG_R14) regs.r14 = variant.var.v_uint64;
            else if (reg == XREG_R15) regs.r15 = variant.var.v_uint64;
            else if (reg == XREG_RIP) regs.rip = variant.var.v_uint64;
            else if (reg == XREG_RFLAGS) regs.eflags = variant.var.v_uint64;
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
    bResult = setCurrentRegByThread(g_statusCurrent.hThread, reg, variant);
#endif
#ifdef Q_OS_LINUX
    bResult = setCurrentRegById(g_statusCurrent.nThreadId, reg, variant);
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
QList<XProcess::THREAD_INFO> *XInfoDB::getCurrentThreadsList()
{
    return &(g_statusCurrent.listThreads);
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::addBreakPoint(const BREAKPOINT &breakPoint)
{
    BREAKPOINT _breakPoint = breakPoint;

    bool bResult = false;

    if ((_breakPoint.bpType == BPT_CODE_SOFTWARE_DEFAULT) || (_breakPoint.bpType == BPT_UNKNOWN)) {
        _breakPoint.bpType = g_bpTypeDefault;
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

        g_listBreakpoints.append(_breakPoint);

        if (enableBreakPoint(_breakPoint.sUUID)) {
            bResult = true;
        } else {
            g_listBreakpoints.removeLast();
        }
    } else if ((_breakPoint.bpType == BPT_CODE_STEP_TO_RESTORE) || (_breakPoint.bpType == BPT_CODE_STEP_FLAG)) {
        bResult = _setStep_Id(_breakPoint.nThreadID);
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::removeBreakPoint(QString sUUID)
{
    bool bResult = false;

    if (disableBreakPoint(sUUID)) {
        qint32 nNumberOfRecords = g_listBreakpoints.count();

        for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
            if (g_listBreakpoints.at(i).sUUID == sUUID) {
                g_listBreakpoints.removeAt(i);

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

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if ((g_listBreakpoints.at(i).nAddress == breakPoint.nAddress) && (g_listBreakpoints.at(i).bpType == breakPoint.bpType) &&
            (g_listBreakpoints.at(i).nThreadID == breakPoint.nThreadID)) {
            bResult = true;
            break;
        }
    }
    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::enableBreakPoint(QString sUUID)
{
    bool bResult = false;

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (g_listBreakpoints.at(i).sUUID == sUUID) {
            if ((g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_HLT) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_CLI) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_STI) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSB) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSD) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSB) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSD) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1LONG) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3LONG) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD0) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD2)) {
                if (read_array(g_listBreakpoints.at(i).nAddress, g_listBreakpoints[i].origData, g_listBreakpoints.at(i).nDataSize) == g_listBreakpoints.at(i).nDataSize) {
                    if (write_array(g_listBreakpoints.at(i).nAddress, (char *)g_listBreakpoints.at(i).bpData, g_listBreakpoints.at(i).nDataSize)) {
                        bResult = true;
                    }
                }
            } else if ((g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR0) || (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR1) ||
                       (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR2) || (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR3)) {
                // TODO
            } else if ((g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_FLAG) || (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_TO_RESTORE)) {
                bResult = _setStep_Id(g_listBreakpoints.at(i).nThreadID);
            }

            break;
        }
    }

    return bResult;
}
#endif
#ifdef USE_XPROCESS
bool XInfoDB::disableBreakPoint(QString sUUID)
{
    bool bResult = false;

    qint32 nNumberOfRecords = g_listBreakpoints.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (g_listBreakpoints.at(i).sUUID == sUUID) {
            if ((g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_HLT) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_CLI) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_STI) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSB) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INSD) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSB) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_OUTSD) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT1LONG) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_INT3LONG) || (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD0) ||
                (g_listBreakpoints.at(i).bpType == BPT_CODE_SOFTWARE_UD2)) {
                if (write_array(g_listBreakpoints.at(i).nAddress, (char *)g_listBreakpoints.at(i).origData, g_listBreakpoints.at(i).nDataSize)) {
                    bResult = true;
                }
            } else if ((g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR0) || (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR1) ||
                       (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR2) || (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_HARDWARE_DR3)) {
                // TODO
            } else if ((g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_FLAG) || (g_listBreakpoints.at(i).bpType == XInfoDB::BPT_CODE_STEP_TO_RESTORE)) {
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
    return &g_listBreakpoints;
}
#endif
#ifdef USE_XPROCESS
#ifdef Q_OS_LINUX
QMap<X_ID, XInfoDB::BREAKPOINT> *XInfoDB::getThreadBreakpoints()
{
    return &g_mapThreadBreakpoints;
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
    nResult = getCurrentRegCache(XInfoDB::XREG_ESP).var.v_uint32;
#endif
#ifdef Q_PROCESSOR_X86_64
    nResult = getCurrentRegCache(XInfoDB::XREG_RSP).var.v_uint64;
#endif

    return nResult;
}
#endif
#ifdef USE_XPROCESS
XADDR XInfoDB::getCurrentInstructionPointerCache()
{
    XADDR nResult = 0;

#ifdef Q_PROCESSOR_X86_32
    nResult = getCurrentRegCache(XInfoDB::XREG_EIP).var.v_uint32;
#endif
#ifdef Q_PROCESSOR_X86_64
    nResult = getCurrentRegCache(XInfoDB::XREG_RIP).var.v_uint64;
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
#ifdef USE_XPROCESS
XCapstone::OPCODE_ID XInfoDB::getCurrentOpcode_Handle(X_HANDLE hThread)
{
    Q_UNUSED(hThread)
    XCapstone::OPCODE_ID result = {};

    // TODO

    return result;
}
#endif
#ifdef USE_XPROCESS
XCapstone::OPCODE_ID XInfoDB::getCurrentOpcode_Id(X_ID nThreadId)
{
    Q_UNUSED(nThreadId)
    XCapstone::OPCODE_ID result = {};

    // TODO

    return result;
}
#endif
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
    RECORD_INFO result = {};

    if ((nValue >= g_nMainModuleAddress) && (nValue < (g_nMainModuleAddress + g_nMainModuleSize))) {
        result.bValid = true;
        result.sModule = g_sMainModuleName;
        result.nAddress = nValue;
    }
#ifdef USE_XPROCESS
    else {
        // TODO mapRegions
        XProcess::MEMORY_REGION memoryRegion = XProcess::getMemoryRegionByAddress(&(g_statusCurrent.listMemoryRegions), nValue);

        if (memoryRegion.nSize) {
            result.bValid = true;
            result.nAddress = nValue;

            XProcess::MODULE _module = XProcess::getModuleByAddress(&(g_statusCurrent.listModules), nValue);

            if (_module.nSize) {
                result.sModule = _module.sName;
            }
        }
    }
#endif

    if ((riType == RI_TYPE_GENERAL) || (riType == RI_TYPE_DATA) || (riType == RI_TYPE_ANSI) || (riType == RI_TYPE_UNICODE) || (riType == RI_TYPE_UTF8)) {
        if (result.bValid) {
#ifdef USE_XPROCESS
            result.baData = read_array(result.nAddress, 64);  // TODO const
#else
            qint64 nCurrentOffset = XBinary::addressToOffset(&g_MainModuleMemoryMap, result.nAddress);
            result.baData = read_array(nCurrentOffset, 64);  // TODO const
#endif
        }
    }

    if ((riType == RI_TYPE_GENERAL) || (riType == RI_TYPE_SYMBOL)) {
        if (result.bValid) {
            result.sSymbol = getSymbolStringByAddress(result.nAddress);

            if (riType == RI_TYPE_SYMBOL) {
                if (result.sSymbol == "") {
                    result.sSymbol = QString("<%1.%2>").arg(result.sModule, XBinary::valueToHexOS(result.nAddress));
                }
            }
        }
    }

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
                sUnicodeString = QString::fromUtf16((quint16 *)(recordInfo.baData.data()), recordInfo.baData.size() / 2);
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
            sResult = QString::fromUtf16((quint16 *)(recordInfo.baData.data()), recordInfo.baData.size() / 2);
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

void XInfoDB::clearRecordInfoCache()
{
    g_mapSRecordInfoCache.clear();
}

XInfoDB::RECORD_INFO XInfoDB::getRecordInfoCache(quint64 nValue)
{
    RECORD_INFO result = {};

    if (g_mapSRecordInfoCache.contains(nValue)) {
        result = g_mapSRecordInfoCache.value(nValue);
    } else {
        result = getRecordInfo(nValue, RI_TYPE_GENERAL);

        g_mapSRecordInfoCache.insert(nValue, result);
    }

    return result;
}

bool XInfoDB::isSymbolsPresent()
{
    // TODO Check count
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1'").arg(s_sql_tableName[DBTABLE_SYMBOLS]), false);

    bResult = query.next();

    if (bResult) {
        querySQL(&query, QString("SELECT count(*) FROM '%1'").arg(s_sql_tableName[DBTABLE_SYMBOLS]), false);

        if (query.next()) {
            bResult = query.value(0).toULongLong();
        }
    }
#else
    bResult = !(g_listSymbols.isEmpty());
#endif

    return bResult;
}

QList<XInfoDB::SYMBOL> XInfoDB::getAllSymbols()
{
    QList<SYMBOL> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT ADDRESS, MODULE, SYMTEXT, SYMSOURCE FROM %1").arg(s_sql_tableName[DBTABLE_SYMBOLS]), false);

    while (query.next()) {
        SYMBOL record = {};

        record.nAddress = query.value(0).toULongLong();
        record.nModule = query.value(1).toULongLong();
        record.sSymbol = query.value(2).toString();
        record.symSource = (SS)query.value(3).toULongLong();

        listResult.append(record);
    }
#else
    listResult = g_listSymbols;
#endif
    return listResult;
}

QMap<quint32, QString> XInfoDB::getSymbolModules()
{
    return g_mapSymbolModules;
}

QList<XInfoDB::REFERENCE> XInfoDB::getReferencesForAddress(XADDR nAddress)
{
    QList<REFERENCE> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS FROM %1 WHERE ((XREFTORELATIVE = %2) OR (XREFTOMEMORY = %2)) AND ((RELTYPE <> 0) OR (MEMTYPE <> 0))")
                 .arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(nAddress)),
             false);

    while (query.next()) {
        REFERENCE record = {};

        record.nAddress = query.value(0).toULongLong();

        SHOWRECORD showRecord = getShowRecordByAddress(record.nAddress, false);

        if (showRecord.nOffset != -1) {
            QByteArray baBuffer = read_array(showRecord.nOffset, showRecord.nSize);
            XCapstone::DISASM_RESULT _disasmResult =
                XCapstone::disasm_ex(g_handle, getDisasmMode(), XBinary::SYNTAX_DEFAULT, baBuffer.data(), baBuffer.size(), record.nAddress);
            record.sCode = _disasmResult.sMnemonic;
            if (_disasmResult.sString != "") {
                record.sCode += " " + convertOpcodeString(_disasmResult, XBinary::SYNTAX_DEFAULT, RI_TYPE_SYMBOLADDRESS);
            }
        }

        listResult.append(record);
    }
#else
    Q_UNUSED(nAddress)
#endif
    return listResult;
}

// QList<XADDR> XInfoDB::getSymbolAddresses(ST symbolType)
//{
//     QList<XADDR> listResult;
// #ifdef QT_SQL_LIB
//     QSqlQuery query(g_dataBase);

//    querySQL(&query, QString("SELECT ADDRESS FROM %1 WHERE SYMTYPE = '%2'").arg(s_sql_symbolTableName, QString::number(symbolType)));

//    while (query.next()) {
//        XADDR nAddress = query.value(0).toULongLong();

//        listResult.append(nAddress);
//    }
// #endif
//    return listResult;
//}

bool XInfoDB::_addSymbol(XADDR nAddress, quint32 nModule, const QString &sSymbol, SS symSource)
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    query.prepare(QString("INSERT INTO %1 (ADDRESS, MODULE, SYMTEXT, SYMSOURCE) "
                          "VALUES (?, ?, ?, ?)")
                      .arg(s_sql_tableName[DBTABLE_SYMBOLS]));

    query.bindValue(0, nAddress);
    query.bindValue(1, nModule);
    query.bindValue(2, sSymbol);
    query.bindValue(3, symSource);

    bResult = querySQL(&query, true);
#else
    SYMBOL symbol = {};
    symbol.nAddress = nAddress;
    symbol.nModule = nModule;
    symbol.sSymbol = sSymbol;
    symbol.symSource = symSource;

    g_listSymbols.append(symbol);

    bResult = true;
#endif
    return bResult;
}

void XInfoDB::_sortSymbols()
{
#ifndef QT_SQL_LIB
    std::sort(g_listSymbols.begin(), g_listSymbols.end(), _symbolSort);
#endif
}

qint32 XInfoDB::_getSymbolIndex(XADDR nAddress, qint64 nSize, quint32 nModule, qint32 *pnInsertIndex)
{
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)
    Q_UNUSED(nModule)
    Q_UNUSED(pnInsertIndex)
    // For sorted g_listSymbols!
    qint32 nResult = -1;
#ifndef QT_SQL_LIB
    //    qint32 nNumberOfRecords = g_listSymbols.count();

    //    for (qint32 i = 0; i < nNumberOfRecords; i++) {
    //        if ((g_listSymbols.at(i).nAddress == nAddress) && (g_listSymbols.at(i).nSize == nSize) && (g_listSymbols.at(i).nModule == nModule)) {
    //            nResult = i;

    //            break;
    //        } else if (g_listSymbols.at(i).nAddress < nAddress) {
    //            *pnInsertIndex = i;

    //            break;
    //        }
    //    }

#endif
    return nResult;
}

bool XInfoDB::_addExportSymbol(XADDR nAddress, const QString &sSymbol)
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);
    query.prepare(QString("INSERT INTO %1 (ADDRESS, ORIGNAME) "
                          "VALUES (?, ?)")
                      .arg(s_sql_tableName[DBTABLE_EXPORT]));

    query.bindValue(0, nAddress);
    query.bindValue(1, sSymbol);

    bResult = querySQL(&query, true);
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(sSymbol)
#endif
    return bResult;
}

bool XInfoDB::_addImportSymbol(XADDR nAddress, const QString &sSymbol)
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);
    query.prepare(QString("INSERT INTO %1 (ADDRESS, ORIGNAME) "
                          "VALUES (?, ?)")
                      .arg(s_sql_tableName[DBTABLE_IMPORT]));

    query.bindValue(0, nAddress);
    query.bindValue(1, sSymbol);

    bResult = querySQL(&query, true);
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(sSymbol)
#endif
    return bResult;
}

bool XInfoDB::_addTLSSymbol(XADDR nAddress, const QString &sSymbol)
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);
    query.prepare(QString("INSERT INTO %1 (ADDRESS, ORIGNAME) "
                          "VALUES (?, ?)")
                      .arg(s_sql_tableName[DBTABLE_TLS]));

    query.bindValue(0, nAddress);
    query.bindValue(1, sSymbol);

    bResult = querySQL(&query, true);
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(sSymbol)
#endif
    return bResult;
}

// QString XInfoDB::symbolSourceIdToString(SS symbolSource)
//{
//     QString sResult = tr("Unknown");

//    if (symbolSource == SS_FILE)
//        sResult = tr("File");
//    else if (symbolSource == SS_USER)
//        sResult = tr("User");

//    return sResult;
//}

// QString XInfoDB::symbolTypeIdToString(ST symbolType)
//{
//     QString sResult = tr("Unknown");

//    if (symbolType == ST_LABEL)
//        sResult = tr("Label");
//    else if (symbolType == ST_LABEL_ENTRYPOINT)
//        sResult = tr("Entry point");
//    else if (symbolType == ST_FUNCTION_EXPORT)
//        sResult = tr("Export");
//    else if (symbolType == ST_FUNCTION_IMPORT)
//        sResult = tr("Import");
//    else if (symbolType == ST_FUNCTION_TLS)
//        sResult = QString("TLS");
//    else if (symbolType == ST_DATA)
//        sResult = tr("Data");
//    else if (symbolType == ST_OBJECT)
//        sResult = tr("Object");
//    else if (symbolType == ST_FUNCTION)
//        sResult = tr("Function");

//    return sResult;
//}

XInfoDB::SYMBOL XInfoDB::getSymbolByAddress(XADDR nAddress)
{
    SYMBOL result = {};
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT ADDRESS, MODULE, SYMTEXT,SYMSOURCE FROM %1 WHERE ADDRESS = '%2'").arg(s_sql_tableName[DBTABLE_SYMBOLS], QString::number(nAddress)),
             false);

    if (query.next()) {
        result.nAddress = query.value(0).toULongLong();
        result.nModule = query.value(1).toULongLong();
        result.sSymbol = query.value(2).toString();
        result.symSource = (SS)query.value(3).toULongLong();
    }
#else
    qint32 nNumberOfSymbols = g_listSymbols.count();

    for (qint32 i = 0; i < nNumberOfSymbols; i++) {
        if (g_listSymbols.at(i).nAddress == nAddress) {
            result = g_listSymbols.at(i);
            break;
        }
    }
#endif
    return result;
}

bool XInfoDB::isSymbolPresent(XADDR nAddress)
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT ADDRESS FROM %1 WHERE ADDRESS = '%2'").arg(s_sql_tableName[DBTABLE_SYMBOLS], QString::number(nAddress)), false);

    bResult = query.next();
#else
    Q_UNUSED(nAddress)
#endif
    return bResult;
}

QList<XInfoDB::FUNCTION> XInfoDB::getAllFunctions()
{
    QList<FUNCTION> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT ADDRESS, SIZE, NAME FROM %1").arg(s_sql_tableName[DBTABLE_FUNCTIONS]), false);

    while (query.next()) {
        FUNCTION record = {};

        record.nAddress = query.value(0).toULongLong();
        record.nSize = query.value(1).toLongLong();
        record.sName = query.value(2).toString();

        listResult.append(record);
    }
#endif
    return listResult;
}

bool XInfoDB::isFunctionPresent(XADDR nAddress)
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT ADDRESS FROM %1 WHERE ADDRESS = '%2'").arg(s_sql_tableName[DBTABLE_FUNCTIONS], QString::number(nAddress)), false);

    bResult = query.next();
#else
    Q_UNUSED(nAddress)
#endif
    return bResult;
}

QString XInfoDB::getSymbolStringByAddress(XADDR nAddress)
{
    // TODO if sql
    QString sResult;

    SYMBOL symbol = getSymbolByAddress(nAddress);

    sResult = symbol.sSymbol;

    return sResult;
}

void XInfoDB::initDisasmDb()
{
#ifdef QT_SQL_LIB
    createTable(&g_dataBase, DBTABLE_SYMBOLS);
    createTable(&g_dataBase, DBTABLE_SHOWRECORDS);
    createTable(&g_dataBase, DBTABLE_RELATIVS);
    createTable(&g_dataBase, DBTABLE_IMPORT);
    createTable(&g_dataBase, DBTABLE_EXPORT);
    createTable(&g_dataBase, DBTABLE_TLS);
    createTable(&g_dataBase, DBTABLE_FUNCTIONS);
    createTable(&g_dataBase, DBTABLE_BOOKMARKS);
    createTable(&g_dataBase, DBTABLE_COMMENTS);
#endif
}

void XInfoDB::initHexDb()
{
#ifdef QT_SQL_LIB
    createTable(&g_dataBase, DBTABLE_BOOKMARKS);
    createTable(&g_dataBase, DBTABLE_COMMENTS);
#endif
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
bool XInfoDB::isTableNotEmpty(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    bool bResult = false;
    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("SELECT count(*) FROM '%1'").arg(s_sql_tableName[dbTable]), false);

    if (query.next()) {
        bResult = (query.value(0).toInt() != 0);
    }

    return bResult;
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

    if (dbTable == DBTABLE_SYMBOLS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "ADDRESS INTEGER PRIMARY KEY,"
                         "MODULE INTEGER,"
                         "SYMTEXT TEXT,"
                         "SYMSOURCE INTEGER"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_SYMBOLS]),
                 false);
    } else if (dbTable == DBTABLE_SHOWRECORDS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "ADDRESS INTEGER PRIMARY KEY,"
                         "ROFFSET INTEGER,"
                         "SIZE INTEGER,"
                         "RECTYPE INTEGER,"
                         "REFTO INTEGER,"
                         "REFFROM INTEGER,"
                         "BRANCH INTEGER,"
                         "DBSTATUS INTEGER"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_SHOWRECORDS]),
                 false);
    } else if (dbTable == DBTABLE_RELATIVS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "ADDRESS INTEGER PRIMARY KEY,"
                         "RELTYPE INTEGER,"
                         "XREFTORELATIVE INTEGER,"
                         "MEMTYPE INTEGER,"
                         "XREFTOMEMORY INTEGER,"
                         "MEMORYSIZE INTEGER,"
                         "DBSTATUS INTEGER"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_RELATIVS]),
                 false);
    } else if (dbTable == DBTABLE_IMPORT) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "ADDRESS INTEGER PRIMARY KEY,"
                         "ORIGNAME TEXT"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_IMPORT]),
                 false);
    } else if (dbTable == DBTABLE_EXPORT) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "ADDRESS INTEGER PRIMARY KEY,"
                         "ORIGNAME TEXT"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_EXPORT]),
                 false);
    } else if (dbTable == DBTABLE_TLS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "ADDRESS INTEGER PRIMARY KEY,"
                         "ORIGNAME TEXT"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_TLS]),
                 false);
    } else if (dbTable == DBTABLE_BOOKMARKS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "UUID TEXT PRIMARY KEY,"
                         "LOCATION INTEGER,"
                         "LOCTYPE INTEGER,"
                         "SIZE INTEGER,"
                         "COLTEXT TEXT,"
                         "COLBACKGROUND TEXT,"
                         "TEMPLATE,"
                         "COMMENT TEXT"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_BOOKMARKS]),
                 false);  // mb column BASE for share objects
    } else if (dbTable == DBTABLE_COMMENTS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "LOCATION INTEGER PRIMARY KEY,"
                         "COMMENT TEXT"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_COMMENTS]),
                 false);  // mb column BASE for share objects
    } else if (dbTable == DBTABLE_FUNCTIONS) {
        querySQL(&query,
                 QString("CREATE TABLE IF NOT EXISTS %1 ("
                         "ADDRESS INTEGER PRIMARY KEY,"
                         "SIZE INTEGER,"
                         "NAME TEXT"
                         ")")
                     .arg(s_sql_tableName[DBTABLE_FUNCTIONS]),
                 false);
    }

    // TODO PDB
    // TODO DWARF
}
#endif
#ifdef QT_SQL_LIB
void XInfoDB::removeTable(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("DROP TABLE IF EXISTS %1").arg(s_sql_tableName[dbTable]), false);
}
#endif
#ifdef QT_SQL_LIB
void XInfoDB::clearTable(QSqlDatabase *pDatabase, DBTABLE dbTable)
{
    QSqlQuery query(*pDatabase);

    querySQL(&query, QString("DELETE FROM TABLE %1").arg(s_sql_tableName[dbTable]), false);
}
#endif
bool XInfoDB::isDbPresent()
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    if (g_dataBase.isOpen()) {
        bResult = isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_BOOKMARKS) || isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_COMMENTS) ||
                  isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_SHOWRECORDS) || isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_RELATIVS) ||
                  isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_IMPORT) || isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_EXPORT) ||
                  isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_TLS) || isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_SYMBOLS) ||
                  isTablePresentAndNotEmpty(&g_dataBase, DBTABLE_FUNCTIONS);
    }
#endif
    return bResult;
}

void XInfoDB::removeAllTables()
{
#ifdef QT_SQL_LIB
    removeTable(&g_dataBase, DBTABLE_BOOKMARKS);
    removeTable(&g_dataBase, DBTABLE_COMMENTS);
    removeTable(&g_dataBase, DBTABLE_SHOWRECORDS);
    removeTable(&g_dataBase, DBTABLE_RELATIVS);
    removeTable(&g_dataBase, DBTABLE_IMPORT);
    removeTable(&g_dataBase, DBTABLE_EXPORT);
    removeTable(&g_dataBase, DBTABLE_TLS);
    removeTable(&g_dataBase, DBTABLE_SYMBOLS);
    removeTable(&g_dataBase, DBTABLE_FUNCTIONS);

    clearRecordInfoCache();
#endif
}

void XInfoDB::clearAllTables()
{
#ifdef QT_SQL_LIB
    clearTable(&g_dataBase, DBTABLE_BOOKMARKS);
    clearTable(&g_dataBase, DBTABLE_COMMENTS);
    clearTable(&g_dataBase, DBTABLE_SHOWRECORDS);
    clearTable(&g_dataBase, DBTABLE_RELATIVS);
    clearTable(&g_dataBase, DBTABLE_IMPORT);
    clearTable(&g_dataBase, DBTABLE_EXPORT);
    clearTable(&g_dataBase, DBTABLE_TLS);
    clearTable(&g_dataBase, DBTABLE_SYMBOLS);
    clearTable(&g_dataBase, DBTABLE_FUNCTIONS);

    clearRecordInfoCache();
#endif
}
void XInfoDB::clearDb()
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("PRAGMA writable_schema = 1"), false);
    querySQL(&query, QString("delete from sqlite_master where type in ('table', 'index', 'trigger')"), false);
    querySQL(&query, QString("PRAGMA writable_schema = 0"), false);

    querySQL(&query, QString("VACUUM"), false);
    querySQL(&query, QString("PRAGMA INTEGRITY_CHECK"), false);

    clearRecordInfoCache();
#endif
}

void XInfoDB::vacuumDb()
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("VACUUM"), false);
#endif
}

void XInfoDB::_addSymbolsFromFile(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct)
{
    g_pMutexSQL->lock();

#ifdef QT_SQL_LIB
    {
        createTable(&g_dataBase, DBTABLE_SYMBOLS);
        createTable(&g_dataBase, DBTABLE_IMPORT);
        createTable(&g_dataBase, DBTABLE_EXPORT);
        createTable(&g_dataBase, DBTABLE_TLS);
    }

    {
        QSqlQuery query(g_dataBase);

        querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_IMPORT]), true);
        querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_EXPORT]), true);
        querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_TLS]), true);
        querySQL(&query, QString("DELETE FROM %1 WHERE SYMSOURCE = '%2'").arg(s_sql_tableName[DBTABLE_SYMBOLS], QString::number(SS_FILE)), true);
    }
#endif

    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

    if (fileType == XBinary::FT_UNKNOWN) {
        fileType = XBinary::getPrefFileType(pDevice);
    }

#ifdef QT_SQL_LIB
    g_dataBase.transaction();
#endif

    // TODO progressBar
    if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) {
        XELF elf(pDevice, bIsImage, nModuleAddress);

        if (elf.isValid()) {
            XBinary::_MEMORY_MAP memoryMap = elf.getMemoryMap();

            if (memoryMap.nEntryPointAddress) {
                _addSymbol(memoryMap.nEntryPointAddress, 0, "EntryPoint", SS_FILE);
                _addFunction(memoryMap.nEntryPointAddress, 0, "EntryPoint");
            }

            {
                QList<XELF_DEF::Elf_Phdr> listProgramHeaders = elf.getElf_PhdrList();

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

            XBinary::_MEMORY_MAP memoryMap = pe.getMemoryMap();

            _addSymbol(memoryMap.nEntryPointAddress, 0, "EntryPoint", SS_FILE);  // TD mb tr
            _addFunction(memoryMap.nEntryPointAddress, 0, "EntryPoint");

            stAddresses.insert(memoryMap.nEntryPointAddress);

            {
                XPE::EXPORT_HEADER _export = pe.getExport(&memoryMap, false, pPdStruct);

                qint32 nNumberOfRecords = _export.listPositions.count();

                for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
                    XADDR nAddress = _export.listPositions.at(i).nAddress;

                    if (!stAddresses.contains(nAddress)) {
                        QString sFunctionName = _export.listPositions.at(i).sFunctionName;
                        if (sFunctionName == "") {
                            sFunctionName = QString::number(_export.listPositions.at(i).nOrdinal);
                        }

                        _addSymbol(nAddress, 0, sFunctionName, SS_FILE);
                        stAddresses.insert(nAddress);

                        _addExportSymbol(nAddress, _export.listPositions.at(i).sFunctionName);  // TODO ordinals
                        _addFunction(nAddress, 0, _export.listPositions.at(i).sFunctionName);
                    }
                }
            }
            {
                QList<XPE::IMPORT_RECORD> listImportRecords = pe.getImportRecords(&memoryMap, pPdStruct);

                qint32 nNumberOfRecords = listImportRecords.count();

                for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
                    XADDR nAddress = XBinary::relAddressToAddress(&memoryMap, listImportRecords.at(i).nRVA);

                    if (!stAddresses.contains(nAddress)) {
                        QString sFunctionName = listImportRecords.at(i).sLibrary + "#" + listImportRecords.at(i).sFunction;
                        _addSymbol(nAddress, 0, sFunctionName, SS_FILE);
                        stAddresses.insert(nAddress);

                        _addImportSymbol(nAddress, listImportRecords.at(i).sFunction);  // TODO ordinals
                        //_addFunction(nAddress, 0, listImportRecords.at(i).sFunction);
                    }
                }
            }
            {
                QList<XADDR> listTLSFunctions = pe.getTLS_CallbacksList(&memoryMap, pPdStruct);

                qint32 nNumberOfRecords = listTLSFunctions.count();

                for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
                    XADDR nAddress = listTLSFunctions.at(i);

                    if (!stAddresses.contains(nAddress)) {
                        QString sFunctionName = QString("tls_sub_%1").arg(XBinary::valueToHexEx(listTLSFunctions.at(i)));
                        _addSymbol(nAddress, 0, sFunctionName, SS_FILE);
                        stAddresses.insert(nAddress);

                        _addTLSSymbol(nAddress, sFunctionName);
                        _addFunction(nAddress, 0, sFunctionName);
                    }
                }
            }
            // TODO PDB
            // TODO DWARF
            // TODO More
        }
    } else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) {
        XMACH mach(pDevice, bIsImage, nModuleAddress);

        if (mach.isValid()) {
            QSet<XADDR> stAddresses;

            XBinary::_MEMORY_MAP memoryMap = mach.getMemoryMap();

            _addSymbol(memoryMap.nEntryPointAddress, 0, "EntryPoint", SS_FILE);  // TD mb tr
            _addFunction(memoryMap.nEntryPointAddress, 0, "EntryPoint");

            stAddresses.insert(memoryMap.nEntryPointAddress);
        }
    }

#ifdef QT_SQL_LIB
    g_dataBase.commit();
    vacuumDb();
#endif

    _sortSymbols();

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    g_pMutexSQL->unlock();
}

void XInfoDB::_addELFSymbols(XELF *pELF, XBinary::_MEMORY_MAP *pMemoryMap, qint64 nDataOffset, qint64 nDataSize, qint64 nStringsTableOffset, qint64 nStringsTableSize,
                             XBinary::PDSTRUCT *pPdStruct)
{
    QList<XELF_DEF::Elf_Sym> listSymbols = pELF->getElf_SymList(nDataOffset, nDataSize);
    qint32 nNumberOfSymbols = listSymbols.count();

    for (int i = 0; (i < nNumberOfSymbols) && (!(pPdStruct->bIsStop)); i++) {
        XELF_DEF::Elf_Sym record = listSymbols.at(i);

        XADDR nSymbolAddress = record.st_value;
        quint64 nSymbolSize = record.st_size;

        qint32 nBind = S_ELF64_ST_BIND(record.st_info);
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
                        _addSymbol(nSymbolAddress, 0, sSymbolName, SS_FILE);
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
    // mb TODO get all symbol info if init
    bool bResult = false;
#ifdef QT_SQL_LIB
    g_pMutexSQL->lock();
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

    XBinary::DM disasmMode = getDisasmMode();
    XBinary::DMFAMILY dmFamily = XBinary::getDisasmFamily(disasmMode);
    // XBinary::MODE mode = XBinary::getModeFromDisasmMode(disasmMode);

    XCapstone::DISASM_OPTIONS disasmOptions = {};

    char byte_buffer[16];  // TODO const
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

    if (analyzeOptions.bIsInit) {
        //_addSymbols(analyzeOptions.pDevice, analyzeOptions.pMemoryMap->fileType, pPdStruct);

        QList<XADDR> listFunctionAddresses;

        listFunctionAddresses.append(getFunctionAddresses());

        qint32 nNumberOfRecords = listFunctionAddresses.count();

        for (qint32 i = 0; (!(pPdStruct->bIsStop)) && (i < nNumberOfRecords); i++) {
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
        if (!(pPdStruct->bIsStop)) {
#ifdef QT_SQL_LIB
            g_dataBase.transaction();
#endif
            QSqlQuery query(g_dataBase);

            QList<XADDR> listImportAddresses = getImportSymbolAddresses();
            qint32 nNumberOfRecords = listImportAddresses.count();
            qint32 nSize = 4;
            //            QString sVarName = "db";
            //            if (mode == XBinary::MODE_16) {
            //                nSize = 2;
            //                sVarName = "word";
            //            } else if (mode == XBinary::MODE_32) {
            //                nSize = 4;
            //                sVarName = "dword";
            //            } else if (mode == XBinary::MODE_64) {
            //                nSize = 8;
            //                sVarName = "qword";
            //            }

            for (qint32 i = 0; (!(pPdStruct->bIsStop)) && (i < nNumberOfRecords); i++) {
                XADDR nAddress = listImportAddresses.at(i);
                if (!_isShowRecordPresent(&query, nAddress, nSize)) {
                    qint64 nOffset = XBinary::addressToOffset(analyzeOptions.pMemoryMap, nAddress);

                    SHOWRECORD showRecord = {};
                    showRecord.nAddress = nAddress;
                    showRecord.nOffset = nOffset;
                    showRecord.nSize = nSize;
                    showRecord.recordType = RT_DATA;
                    showRecord.nRefTo = 0;
                    showRecord.nRefFrom = 0;
                    showRecord.nBranch = 0;
                    showRecord.dbstatus = DBSTATUS_PROCESS;
                    _addShowRecord(showRecord);
                }
            }

#ifdef QT_SQL_LIB
            g_dataBase.commit();
#endif
        }
        // XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, tr(""));
        if (!(pPdStruct->bIsStop)) {
            vacuumDb();
        }
    }

    qint64 nNumberOfOpcodes = 0;

    QList<SHOWRECORD> listShowRecords;
    QList<RELRECORD> listRelRecords;
    QSet<XADDR> stShowRecords;

    QSqlQuery query(g_dataBase);

    while (!(pPdStruct->bIsStop)) {
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

            XADDR nCurrentAddress = nEntryAddress;

            while (!(pPdStruct->bIsStop)) {
                if ((!stShowRecords.contains(nCurrentAddress)) && (!_isShowRecordPresent(&query, nCurrentAddress, 1))) {
                    qint64 nOffset = XBinary::addressToOffset(analyzeOptions.pMemoryMap, nCurrentAddress);

                    if (nOffset != -1) {
                        qint64 nSize = 16;

                        nSize = qMin(nTotalSize - nOffset, nSize);

                        nSize = binary.read_array(nOffset, byte_buffer, nSize);

                        if (dmFamily == XBinary::DMFAMILY_X86) {
                            if (nSize >= 2) {
                                // mb TODO optional
                                // add byte ptr [rax], al
                                if ((*((quint16 *)byte_buffer) == 0) && (nCurrentAddress != analyzeOptions.nStartAddress)) {
                                    break;
                                }
                            }
                        }

                        if (nSize > 0) {
                            XCapstone::DISASM_RESULT disasmResult =
                                XCapstone::disasm_ex(g_handle, disasmMode, XBinary::SYNTAX_DEFAULT, byte_buffer, nSize, nCurrentAddress, disasmOptions);

                            if (disasmResult.bIsValid) {
                                {
                                    quint64 _nCurrentBranch = nCurrentBranch;

                                    if (bSuspect) {
                                        if (XCapstone::isNopOpcode(dmFamily, disasmResult.nOpcode)) {
                                            _nCurrentBranch = 0;
                                        } else if (XCapstone::isInt3Opcode(dmFamily, disasmResult.nOpcode)) {
                                            _nCurrentBranch = 0;
                                        }
                                    }

                                    quint64 nRefTo = 0;

                                    if (disasmResult.relType) {
                                        nRefTo++;
                                    }

                                    if (disasmResult.memType) {
                                        nRefTo++;
                                    }

                                    SHOWRECORD showRecord = {};
                                    showRecord.bValid = true;
                                    showRecord.nAddress = disasmResult.nAddress;
                                    showRecord.nOffset = nOffset;
                                    showRecord.nSize = disasmResult.nSize;
                                    showRecord.recordType = RT_CODE;
                                    showRecord.nRefTo = nRefTo;
                                    showRecord.nRefFrom = 0;
                                    showRecord.nBranch = _nCurrentBranch;
                                    showRecord.dbstatus = DBSTATUS_PROCESS;
                                    listShowRecords.append(showRecord);

                                    if (disasmResult.relType || disasmResult.memType) {
                                        RELRECORD relRecord = {};
                                        relRecord.nAddress = disasmResult.nAddress;
                                        relRecord.relType = disasmResult.relType;
                                        relRecord.nXrefToRelative = disasmResult.nXrefToRelative;
                                        relRecord.memType = disasmResult.memType;
                                        relRecord.nXrefToMemory = disasmResult.nXrefToMemory;
                                        relRecord.nMemorySize = disasmResult.nMemorySize;
                                        relRecord.dbstatus = DBSTATUS_PROCESS;
                                        listRelRecords.append(relRecord);
                                    }

                                    nNumberOfOpcodes++;

                                    if (analyzeOptions.nCount && (nNumberOfOpcodes > analyzeOptions.nCount)) {
                                        break;
                                    }
                                }

                                stShowRecords.insert(nCurrentAddress);

                                nCurrentAddress += disasmResult.nSize;
                                XBinary::setPdStructCurrentIncrement(pPdStruct, _nFreeIndex);

                                if (disasmResult.relType) {
                                    if (XCapstone::isCallOpcode(dmFamily, disasmResult.nOpcode)) {
                                        nBranch++;
                                        _ENTRY _entry = {};
                                        _entry.nAddress = disasmResult.nXrefToRelative;
                                        _entry.nBranch = nBranch;
                                        listEntries.append(_entry);
                                    } else {
                                        _ENTRY _entry = {};
                                        _entry.nAddress = disasmResult.nXrefToRelative;
                                        _entry.nBranch = nCurrentBranch;
                                        listEntries.append(_entry);
                                    }

                                    if (!(analyzeOptions.bIsInit)) {
                                        // TODO relative if code code
                                    }
                                }

                                if (stShowRecords.count() > 10000) {  // TODO Consts
                                    _ENTRY _entry = {};
                                    _entry.nAddress = nCurrentAddress;
                                    _entry.nBranch = nCurrentBranch;
                                    listEntries.append(_entry);

                                    break;
                                }

                                // TODO Check mb int3
                                if (XCapstone::isRetOpcode(dmFamily, disasmResult.nOpcode)) {
                                    //                                    if ((nCurrentAddress >= mrCode.nAddress) && (nCurrentAddress < (mrCode.nAddress +
                                    //                                    mrCode.nSize))) {
                                    //                                        listSuspect.append(nCurrentAddress);
                                    //                                    }
                                    break;
                                }

                                if (XCapstone::isInt3Opcode(dmFamily, disasmResult.nOpcode)) {
                                    break;
                                }

                                if (XCapstone::isJumpOpcode(dmFamily, disasmResult.nOpcode)) {
                                    break;
                                }
                                //                                if (dmFamily == XBinary::DMFAMILY_X86) {
                                //                                    // TODO Check
                                //                                    if ((disasmResult.sMnemonic == "ret") || (disasmResult.sMnemonic == "retn") ||
                                //                                    (disasmResult.sMnemonic == "jmp")) {
                                //                                        stEntries.insert(nCurrentAddress); // TODO optional
                                //                                        break;
                                //                                    }
                                //                                } else if ((dmFamily == XBinary::DMFAMILY_ARM) || (dmFamily == XBinary::DMFAMILY_ARM64)) {
                                //                                    // TODO Check
                                //                                    if ((disasmResult.sMnemonic == "b")) {
                                //                                        stEntries.insert(nCurrentAddress); // TODO optional
                                //                                        break;
                                //                                    }
                                //                                }
                            } else {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        if (nCurrentAddress == nEntryAddress) {
                            // Virtual
                            SHOWRECORD showRecord = {};
                            showRecord.nAddress = nCurrentAddress;
                            showRecord.nOffset = -1;
                            showRecord.nSize = 1;
                            //                            showRecord.sRecText1 = "db";
                            //                            showRecord.sRecText2 = "0x01 dup (?)";
                            showRecord.recordType = RT_VIRTUAL;
                            showRecord.nRefTo = 0;
                            showRecord.nRefFrom = 0;
                            showRecord.dbstatus = DBSTATUS_PROCESS;
                            listShowRecords.append(showRecord);

                            nNumberOfOpcodes++;

                            if (analyzeOptions.nCount && (nNumberOfOpcodes > analyzeOptions.nCount)) {
                                break;
                            }
                        }

                        break;
                    }
                } else {
                    break;
                }
            }

            if (!bSuspect) {
                if (listEntries.first().nAddress == nEntryAddress) {
                    listEntries.removeFirst();
                }
            } /*else {
                if (listSuspect.first() == nEntryAddress) {
                    listSuspect.removeFirst();
                }
            }*/

            if (listEntries.isEmpty() || (listShowRecords.count() > 10000)) {
#ifdef QT_SQL_LIB
                g_dataBase.transaction();
#endif
                // XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, tr("Save"));
                _addShowRecords(&query, &listShowRecords);
                _addRelRecords(&query, &listRelRecords);
#ifdef QT_SQL_LIB
                g_dataBase.commit();
#endif
                listShowRecords.clear();
                listRelRecords.clear();
                stShowRecords.clear();
            }
        } else {
            break;
        }
    }

    if (!(pPdStruct->bIsStop)) {
        vacuumDb();
    }

    if (!(pPdStruct->bIsStop)) {
#ifdef QT_SQL_LIB
        g_dataBase.transaction();
#endif
        QSet<XADDR> stCalls;

        // Functions

        {
            // Calls
            QList<XADDR> listLabels = getShowRecordRelAddresses(XCapstone::RELTYPE_CALL, DBSTATUS_PROCESS);
            qint32 nNumberOfLabels = listLabels.count();

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
            XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, nNumberOfLabels);

            for (qint32 i = 0; (!(pPdStruct->bIsStop)) && (i < nNumberOfLabels); i++) {
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
            QList<XBinary::ADDRESSSIZE> listBranches = getBranches(DBSTATUS_PROCESS);
            qint32 nNumberOfBranches = listBranches.count();

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
            XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, nNumberOfBranches);

            for (qint32 i = 0; (!(pPdStruct->bIsStop)) && (i < nNumberOfBranches); i++) {
                XADDR nSymbolAddress = listBranches.at(i).nAddress;

                QString sSymbolName;

                if (stCalls.contains(nSymbolAddress)) {
                    sSymbolName = QString("sub_%1").arg(XBinary::valueToHexEx(nSymbolAddress));
                } else {
                    sSymbolName = QString("unk_%1").arg(XBinary::valueToHexEx(nSymbolAddress));
                }

                if (!isSymbolPresent(nSymbolAddress)) {
                    _addSymbol(nSymbolAddress, 0, sSymbolName, SS_ANALYZE);
                }

                if (!isFunctionPresent(nSymbolAddress)) {
                    _addFunction(nSymbolAddress, listBranches.at(i).nSize, sSymbolName);
                } else {
                    updateFunctionSize(nSymbolAddress, listBranches.at(i).nSize);
                }

                XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
            }
        }

#ifdef QT_SQL_LIB
        g_dataBase.commit();
#endif
    }

    // Labels
    if (!(pPdStruct->bIsStop)) {
#ifdef QT_SQL_LIB
        g_dataBase.transaction();
#endif
        QList<XADDR> listLabels = getShowRecordRelAddresses(XCapstone::RELTYPE_ALL, DBSTATUS_PROCESS);
        qint32 nNumberOfLabels = listLabels.count();

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
        XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, nNumberOfLabels);

        for (qint32 i = 0; (!(pPdStruct->bIsStop)) && (i < nNumberOfLabels); i++) {
            XADDR nSymbolAddress = listLabels.at(i);

            if (!isSymbolPresent(nSymbolAddress)) {
                QString sSymbolName = QString("label_%1").arg(XBinary::valueToHexEx(nSymbolAddress));

                _addSymbol(nSymbolAddress, 0, sSymbolName, SS_ANALYZE);
            }

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
        }

#ifdef QT_SQL_LIB
        g_dataBase.commit();
#endif
    }

    if (!(pPdStruct->bIsStop)) {
        vacuumDb();
    }

    // TODO Get all branches
    // TODO Get all functions and check size

    // Variables
    if (!(pPdStruct->bIsStop)) {
#ifdef QT_SQL_LIB
        g_dataBase.transaction();
#endif
        QSqlQuery query(g_dataBase);

        QList<XBinary::ADDRESSSIZE> listVariables = getShowRecordMemoryVariables(DBSTATUS_PROCESS);
        qint32 nNumberOfVariables = listVariables.count();

        for (qint32 i = 0; (!(pPdStruct->bIsStop)) && (i < nNumberOfVariables); i++) {
            XBinary::ADDRESSSIZE record = listVariables.at(i);

            //                // TODO if size = 0 check if it is a string
            //                QString sVarName = "db";
            //                if (record.nSize == 0) {
            //                    record.nSize = 1;
            //                } else if (record.nSize == 2) {
            //                    sVarName = "word";
            //                } else if (record.nSize == 4) {
            //                    sVarName = "dword";
            //                } else if (record.nSize == 8) {
            //                    sVarName = "qword";
            //                }

            bool bAdd = false;

            if (record.nSize) {
                if (!_isShowRecordPresent(&query, record.nAddress, record.nSize)) {
                    bAdd = true;
                }
            }

            if (bAdd) {
                if (XBinary::isAddressValid(analyzeOptions.pMemoryMap, record.nAddress)) {
                    qint64 nOffset = XBinary::addressToOffset(analyzeOptions.pMemoryMap, record.nAddress);

                    SHOWRECORD showRecord = {};
                    showRecord.nAddress = record.nAddress;
                    showRecord.nOffset = nOffset;
                    showRecord.nSize = record.nSize;
                    showRecord.recordType = RT_DATA;
                    showRecord.nRefTo = 0;
                    showRecord.nRefFrom = 0;
                    showRecord.nBranch = 0;
                    showRecord.dbstatus = DBSTATUS_PROCESS;
                    _addShowRecord(showRecord);
                }
            }

            if (!isSymbolPresent(record.nAddress)) {
                if (XBinary::isAddressValid(analyzeOptions.pMemoryMap, record.nAddress)) {
                    QString sSymbolName;

                    // TODO Check string
                    if (record.nSize) {
                        sSymbolName = QString("var_%1").arg(XBinary::valueToHexEx(record.nAddress));
                    } else {
                        sSymbolName = QString("label_%1").arg(XBinary::valueToHexEx(record.nAddress));
                    }

                    if (!_addSymbol(record.nAddress, 0, sSymbolName, SS_ANALYZE)) {
#ifdef QT_DEBUG
                        qDebug("%s", XBinary::valueToHex(record.nAddress).toLatin1().data());
#endif
                    }
                    // TODO ST_DATA_ANSISTRING
                }
            }
        }

#ifdef QT_SQL_LIB
        g_dataBase.commit();
#endif
        // XBinary::setPdStructStatus(pPdStruct, _nFreeIndex, tr(""));
        if (!(pPdStruct->bIsStop)) {
            vacuumDb();
        }
    }

    // Update references
    if (analyzeOptions.bIsInit) {
        if (!(pPdStruct->bIsStop)) {
#ifdef QT_SQL_LIB
            g_dataBase.transaction();
#endif
            QList<RELRECORD> listRelRecords = getRelRecords(DBSTATUS_PROCESS);  // TODO optimize
            // TODO reset counters!!!

            qint32 nNumberOfRecords = listRelRecords.count();

            for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
                RELRECORD record = listRelRecords.at(i);

                if (record.relType) {
                    _incShowRecordRefFrom(record.nXrefToRelative);
                }

                if (record.memType) {
                    _incShowRecordRefFrom(record.nXrefToMemory);
                }
            }
            // TODO

#ifdef QT_SQL_LIB
            g_dataBase.commit();
            if (!(pPdStruct->bIsStop)) {
                vacuumDb();
            }
#endif
        }
    }

    //    if (bIsInit) {
    //        if (!(pPdStruct->bIsStop)) {
    // #ifdef QT_SQL_LIB
    //            g_dataBase.transaction();
    // #endif

    //            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, 0);
    //            XBinary::setPdStructTotal(pPdStruct, _nFreeIndex, pMemoryMap->nImageSize);

    //            // updateShowRecordLine
    //            quint64 nLineNumber = 0;

    //            for (XADDR nCurrentAddress = pMemoryMap->nModuleAddress;
    //                 (!(pPdStruct->bIsStop)) && (nCurrentAddress < (pMemoryMap->nModuleAddress + pMemoryMap->nImageSize));) {
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

    //                    for (XADDR _nCurrentAddress = nCurrentAddress; (!(pPdStruct->bIsStop)) && (_nCurrentAddress < nCurrentAddress + nRecordSize);) {
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
    //                            for (XADDR _nCurrentAddressData = _nCurrentAddress; (!(pPdStruct->bIsStop)) && (_nCurrentAddressData < _nCurrentAddress +
    //                            _nRecordSize);) {
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
    //             if (!(pPdStruct->bIsStop)) {
    //                 vacuumDb();
    //             }
    // #endif
    //         }
    //     }

    // mb TODO Overlay

    _completeDbAnalyze();

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    g_pMutexSQL->unlock();

    if (!pPdStruct->bIsStop) {
        bResult = true;
    }
#else
    Q_UNUSED(analyzeOptions)
    Q_UNUSED(pPdStruct)
#endif

    return bResult;
}

bool XInfoDB::_addShowRecord(const SHOWRECORD &record)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    query.prepare(QString("INSERT INTO %1 (ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?)")
                      .arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));

    query.bindValue(0, record.nAddress);
    query.bindValue(1, record.nOffset);
    query.bindValue(2, record.nSize);
    query.bindValue(3, record.recordType);
    query.bindValue(4, record.nRefTo);
    query.bindValue(5, record.nRefFrom);
    query.bindValue(6, record.nBranch);
    query.bindValue(7, record.dbstatus);

    bResult = querySQL(&query, true);
#else
    Q_UNUSED(record)
#endif

    return bResult;
}
#ifdef QT_SQL_LIB
bool XInfoDB::_isShowRecordPresent(QSqlQuery *pQuery, XADDR nAddress, qint64 nSize)
{
    bool bResult = false;

    if (nSize <= 1) {
        pQuery->prepare(QString("SELECT ADDRESS FROM %1 WHERE ADDRESS = ?").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));
        pQuery->bindValue(0, nAddress);
    } else {
        pQuery->prepare(QString("SELECT ADDRESS FROM %1 WHERE (ADDRESS >= ?) AND (ADDRESS < ?)").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));
        pQuery->bindValue(0, nAddress);
        pQuery->bindValue(1, nAddress + nSize);
    }

    if (querySQL(pQuery, false)) {
        bResult = pQuery->next();
    }

    return bResult;
}
#endif
bool XInfoDB::_addRelRecord(const RELRECORD &record)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    query.prepare(QString("INSERT INTO %1 (ADDRESS, RELTYPE, XREFTORELATIVE, MEMTYPE, XREFTOMEMORY, MEMORYSIZE, DBSTATUS) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?)")
                      .arg(s_sql_tableName[DBTABLE_RELATIVS]));

    query.bindValue(0, record.nAddress);
    query.bindValue(1, record.relType);
    query.bindValue(2, record.nXrefToRelative);
    query.bindValue(3, record.memType);
    query.bindValue(4, record.nXrefToMemory);
    query.bindValue(5, record.nMemorySize);
    query.bindValue(6, record.dbstatus);

    bResult = querySQL(&query, true);
#else
    Q_UNUSED(record)
#endif

    return bResult;
}

void XInfoDB::_completeDbAnalyze()
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("UPDATE %1 SET DBSTATUS = '%2' WHERE DBSTATUS = '%3'")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(DBSTATUS_NONE), QString::number(DBSTATUS_PROCESS)),
             true);
    querySQL(&query,
             QString("UPDATE %1 SET DBSTATUS = '%2' WHERE DBSTATUS = '%3'")
                 .arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(DBSTATUS_NONE), QString::number(DBSTATUS_PROCESS)),
             true);
#endif
}
#ifdef QT_SQL_LIB
void XInfoDB::_addShowRecords(QSqlQuery *pQuery, QList<SHOWRECORD> *pListRecords)
{
    pQuery->prepare(QString("INSERT INTO %1 (ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS) "
                            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)")
                        .arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));

    qint32 nNumberOfRecords = pListRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        pQuery->bindValue(0, pListRecords->at(i).nAddress);
        pQuery->bindValue(1, pListRecords->at(i).nOffset);
        pQuery->bindValue(2, pListRecords->at(i).nSize);
        pQuery->bindValue(3, pListRecords->at(i).recordType);
        pQuery->bindValue(4, pListRecords->at(i).nRefTo);
        pQuery->bindValue(5, pListRecords->at(i).nRefFrom);
        pQuery->bindValue(6, pListRecords->at(i).nBranch);
        pQuery->bindValue(7, pListRecords->at(i).dbstatus);

        querySQL(pQuery, true);
    }
}
#endif
#ifdef QT_SQL_LIB
void XInfoDB::_addRelRecords(QSqlQuery *pQuery, QList<RELRECORD> *pListRecords)
{
    pQuery->prepare(QString("INSERT INTO %1 (ADDRESS, RELTYPE, XREFTORELATIVE, MEMTYPE, XREFTOMEMORY, MEMORYSIZE, DBSTATUS) "
                            "VALUES (?, ?, ?, ?, ?, ?, ?)")
                        .arg(s_sql_tableName[DBTABLE_RELATIVS]));

    qint32 nNumberOfRecords = pListRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        pQuery->bindValue(0, pListRecords->at(i).nAddress);
        pQuery->bindValue(1, pListRecords->at(i).relType);
        pQuery->bindValue(2, pListRecords->at(i).nXrefToRelative);
        pQuery->bindValue(3, pListRecords->at(i).memType);
        pQuery->bindValue(4, pListRecords->at(i).nXrefToMemory);
        pQuery->bindValue(5, pListRecords->at(i).nMemorySize);
        pQuery->bindValue(6, pListRecords->at(i).dbstatus);

        querySQL(pQuery, true);
    }
}
#endif
#ifdef QT_SQL_LIB
quint64 XInfoDB::_getBranchNumber()
{
    quint64 nResult = 0;

    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT MAX(BRANCH) + 1 FROM %1").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]), false);

    if (query.next()) {
        nResult = query.value(0).toULongLong();
    }

    if (nResult == 0) {
        nResult = 1;
    }

    return nResult;
}
#endif
QList<XInfoDB::RELRECORD> XInfoDB::getRelRecords(DBSTATUS dbstatus)
{
    QList<XInfoDB::RELRECORD> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL = QString("SELECT ADDRESS, RELTYPE, XREFTORELATIVE, MEMTYPE, XREFTOMEMORY, MEMORYSIZE, DBSTATUS FROM %1").arg(s_sql_tableName[DBTABLE_RELATIVS]);

    if (dbstatus != DBSTATUS_NONE) {
        sSQL += QString(" WHERE DBSTATUS = '%1'").arg(dbstatus);
    }

    querySQL(&query, sSQL, false);

    while (query.next()) {
        RELRECORD record = {};

        record.nAddress = query.value(0).toULongLong();
        record.relType = (XCapstone::RELTYPE)query.value(1).toULongLong();  // TODO
        record.nXrefToRelative = query.value(2).toULongLong();
        record.memType = (XCapstone::MEMTYPE)query.value(3).toULongLong();  // TODO
        record.nXrefToMemory = query.value(4).toULongLong();
        record.nMemorySize = query.value(5).toLongLong();
        record.dbstatus = (DBSTATUS)query.value(6).toLongLong();

        listResult.append(record);
    }
#else
    Q_UNUSED(dbstatus)
#endif

    return listResult;
}

bool XInfoDB::_incShowRecordRefFrom(XADDR nAddress)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    bResult = querySQL(&query, QString("UPDATE %1 SET REFFROM=REFFROM+1 WHERE ADDRESS=%2").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)), true);
#else
    Q_UNUSED(nAddress)
#endif

    return bResult;
}

bool XInfoDB::_removeAnalyze(XADDR nAddress, qint64 nSize)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    bResult = querySQL(&query,
                       QString("DELETE FROM %1 WHERE ADDRESS >= %2 AND ADDRESS < %3")
                           .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress), QString::number(nAddress + nSize)),
                       true);

    // TODO REMOVE XREFS
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)
#endif

    return bResult;
}

void XInfoDB::_clearAnalyze()
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]), true);
    querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_FUNCTIONS]), true);
    querySQL(&query, QString("DELETE FROM %1").arg(s_sql_tableName[DBTABLE_RELATIVS]), true);
    querySQL(&query, QString("DELETE FROM %1 WHERE SYMSOURCE <> '%2'").arg(s_sql_tableName[DBTABLE_SYMBOLS], QString::number(SS_FILE)), true);
#endif
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

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    query.prepare(QString("INSERT INTO %1 (ADDRESS, SIZE, NAME) "
                          "VALUES (?, ?, ?)")
                      .arg(s_sql_tableName[DBTABLE_FUNCTIONS]));

    query.bindValue(0, nAddress);
    query.bindValue(1, nSize);
    query.bindValue(2, sName);

    bResult = querySQL(&query, true);
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)
    Q_UNUSED(sName)
#endif

    return bResult;
}

void XInfoDB::updateFunctionSize(XADDR nAddress, qint64 nSize)
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("UPDATE %1 SET SIZE = '%2' WHERE ADDRESS = '%3'").arg(s_sql_tableName[DBTABLE_FUNCTIONS], QString::number(nSize), QString::number(nAddress)),
             true);
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)
#endif
}
#ifdef QT_GUI_LIB
QString XInfoDB::_addBookmarkRecord(const BOOKMARKRECORD &record)
{
    QString sResult;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sUUID = record.sUUID;

    if (sUUID == "") {
        sUUID = XBinary::generateUUID();
    }

    query.prepare(QString("INSERT INTO %1 (UUID, LOCATION, LOCTYPE, SIZE, COLTEXT, COLBACKGROUND, TEMPLATE, COMMENT) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?)")
                      .arg(s_sql_tableName[DBTABLE_BOOKMARKS]));

    query.bindValue(0, sUUID);
    query.bindValue(1, record.nLocation);
    query.bindValue(2, record.locationType);
    query.bindValue(3, record.nSize);
    query.bindValue(4, colorToString(record.colText));
    query.bindValue(5, colorToString(record.colBackground));
    query.bindValue(6, record.sTemplate);
    query.bindValue(7, record.sComment);

    if (querySQL(&query, true)) {
        sResult = sUUID;
    }
#else
    Q_UNUSED(record)
#endif

    return sResult;
}
#endif
#ifdef QT_GUI_LIB
bool XInfoDB::_removeBookmarkRecord(const QString &sUUID)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("DELETE FROM %1 WHERE UUID = '%2'").arg(s_sql_tableName[DBTABLE_BOOKMARKS], sUUID), true);

    bResult = querySQL(&query, true);
#else
    Q_UNUSED(sUUID)
#endif

    return bResult;
}
#endif
#ifdef QT_GUI_LIB
QList<XInfoDB::BOOKMARKRECORD> XInfoDB::getBookmarkRecords()
{
    QList<XInfoDB::BOOKMARKRECORD> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT UUID, LOCATION, LOCTYPE, SIZE, COLTEXT, COLBACKGROUND, TEMPLATE, COMMENT FROM %1 ORDER BY LOCATION").arg(s_sql_tableName[DBTABLE_BOOKMARKS]),
             false);

    while (query.next()) {
        BOOKMARKRECORD record = {};

        record.sUUID = query.value(0).toString();
        record.nLocation = query.value(1).toULongLong();
        record.locationType = (LT)query.value(2).toLongLong();
        record.nSize = query.value(3).toLongLong();
        record.colText = stringToColor(query.value(4).toString());
        record.colBackground = stringToColor(query.value(5).toString());
        record.sTemplate = query.value(6).toString();
        record.sComment = query.value(7).toString();

        listResult.append(record);
    }

#endif

    return listResult;
}
#endif
#ifdef QT_GUI_LIB
QList<XInfoDB::BOOKMARKRECORD> XInfoDB::getBookmarkRecords(quint64 nLocation, LT locationType, qint64 nSize)
{
    QList<XInfoDB::BOOKMARKRECORD> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT UUID, LOCATION, LOCTYPE, SIZE, COLTEXT, COLBACKGROUND, TEMPLATE, COMMENT FROM %1 "
                     "WHERE ((%2 + %3) > LOCATION) AND ((LOCATION >= %2) OR ((%2 + %3) < (LOCATION + SIZE))) "
                     "OR ((LOCATION + SIZE) > %2) AND ((%2 >= LOCATION) OR ((LOCATION + SIZE) < (%2 + %3))) AND LOCTYPE = '%4'  ORDER BY LOCATION")
                 .arg(s_sql_tableName[DBTABLE_BOOKMARKS], QString::number(nLocation), QString::number(nSize), QString::number(locationType)),
             false);

    while (query.next()) {
        BOOKMARKRECORD record = {};

        record.sUUID = query.value(0).toString();
        record.nLocation = query.value(1).toULongLong();
        record.locationType = (LT)query.value(2).toLongLong();
        record.nSize = query.value(3).toLongLong();
        record.colText = stringToColor(query.value(4).toString());
        record.colBackground = stringToColor(query.value(5).toString());
        record.sTemplate = query.value(6).toString();
        record.sComment = query.value(7).toString();

        listResult.append(record);
    }
#else
    Q_UNUSED(nLocation)
    Q_UNUSED(locationType)
    Q_UNUSED(nSize)
#endif

    return listResult;
}
#endif
#ifdef QT_GUI_LIB
void XInfoDB::updateBookmarkRecord(BOOKMARKRECORD &record)
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("UPDATE %1 SET "
                     "LOCATION = '%2', "
                     "LOCTYPE = '%3', "
                     "SIZE = '%4', "
                     "COLTEXT = '%5', "
                     "COLBACKGROUND = '%6', "
                     "TEMPLATE = '%7' "
                     "COMMENT = '%8' "
                     "WHERE UUID = '%9'")
                 .arg(s_sql_tableName[DBTABLE_BOOKMARKS], QString::number(record.nLocation), QString::number(record.locationType), QString::number(record.nSize),
                      colorToString(record.colText), colorToString(record.colBackground), record.sTemplate, record.sComment, record.sUUID),
             true);
#else
    Q_UNUSED(record)
#endif
}
#endif
#ifdef QT_GUI_LIB
void XInfoDB::updateBookmarkRecordColor(const QString &sUUID, const QColor &colBackground)
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("UPDATE %1 SET COLBACKGROUND = '%2' WHERE UUID = '%3'").arg(s_sql_tableName[DBTABLE_BOOKMARKS], colorToString(colBackground), sUUID), true);
#else
    Q_UNUSED(sUUID)
    Q_UNUSED(colBackground)
#endif
}
#endif
#ifdef QT_GUI_LIB
void XInfoDB::updateBookmarkRecordComment(const QString &sUUID, const QString &sComment)
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("UPDATE %1 SET COMMENT = '%2' WHERE UUID = '%3'").arg(s_sql_tableName[DBTABLE_BOOKMARKS], convertStringSQLValue(sComment), sUUID), true);
#else
    Q_UNUSED(sUUID)
    Q_UNUSED(sComment)
#endif
}
#endif

XInfoDB::SHOWRECORD XInfoDB::getShowRecordByAddress(XADDR nAddress, bool bIsAprox)
{
    XInfoDB::SHOWRECORD result = {};

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);
    g_pMutexSQL->lock();
    if (!bIsAprox) {
        querySQL(&query,
                 QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE ADDRESS = %2")
                     .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)),
                 false);
    } else {
        querySQL(&query,
                 QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE (ADDRESS <= %2) AND ((ADDRESS + SIZE) > %2)")
                     .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)),
                 false);
    }

    if (query.next()) {
        result.bValid = true;
        result.nAddress = query.value(0).toULongLong();
        result.nOffset = query.value(1).toLongLong();
        result.nSize = query.value(2).toLongLong();
        result.recordType = (RT)query.value(3).toULongLong();
        result.nRefTo = query.value(4).toULongLong();
        result.nRefFrom = query.value(5).toULongLong();
        result.nBranch = query.value(6).toULongLong();
        result.dbstatus = (DBSTATUS)query.value(7).toLongLong();
    }
    g_pMutexSQL->unlock();
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(bAprox)
#endif

    return result;
}

XInfoDB::SHOWRECORD XInfoDB::getNextShowRecordByAddress(XADDR nAddress)
{
    XInfoDB::SHOWRECORD result = {};

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE ADDRESS > '%2' ORDER BY ADDRESS LIMIT 1")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)),
             false);

    if (query.next()) {
        result.bValid = true;
        result.nAddress = query.value(0).toULongLong();
        result.nOffset = query.value(1).toLongLong();
        result.nSize = query.value(2).toLongLong();
        result.recordType = (RT)query.value(3).toULongLong();
        result.nRefTo = query.value(4).toULongLong();
        result.nRefFrom = query.value(5).toULongLong();
        result.nBranch = query.value(6).toULongLong();
        result.dbstatus = (DBSTATUS)query.value(7).toLongLong();
    }
#else
    Q_UNUSED(nAddress)
#endif

    return result;
}

XInfoDB::SHOWRECORD XInfoDB::getPrevShowRecordByAddress(XADDR nAddress)
{
    XInfoDB::SHOWRECORD result = {};

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE ADDRESS < '%2' ORDER BY ADDRESS DESC LIMIT 1")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)),
             false);

    if (query.next()) {
        result.bValid = true;
        result.nAddress = query.value(0).toULongLong();
        result.nOffset = query.value(1).toLongLong();
        result.nSize = query.value(2).toLongLong();
        result.recordType = (RT)query.value(3).toULongLong();
        result.nRefTo = query.value(4).toULongLong();
        result.nRefFrom = query.value(5).toULongLong();
        result.nBranch = query.value(6).toULongLong();
        result.dbstatus = (DBSTATUS)query.value(7).toLongLong();
    }
#else
    Q_UNUSED(nAddress)
#endif

    return result;
}

XInfoDB::SHOWRECORD XInfoDB::getNextShowRecordByOffset(qint64 nOffset)
{
    XInfoDB::SHOWRECORD result = {};

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE ROFFSET > '%2' ORDER BY ROFFSET LIMIT 1")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nOffset)),
             false);

    if (query.next()) {
        result.bValid = true;
        result.nAddress = query.value(0).toULongLong();
        result.nOffset = query.value(1).toLongLong();
        result.nSize = query.value(2).toLongLong();
        result.recordType = (RT)query.value(3).toULongLong();
        result.nRefTo = query.value(4).toULongLong();
        result.nRefFrom = query.value(5).toULongLong();
        result.nBranch = query.value(6).toULongLong();
        result.dbstatus = (DBSTATUS)query.value(7).toLongLong();
    }
#else
    Q_UNUSED(nOffset)
#endif

    return result;
}

XInfoDB::SHOWRECORD XInfoDB::getPrevShowRecordByOffset(qint64 nOffset)
{
    XInfoDB::SHOWRECORD result = {};

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE ROFFSET < '%2' ORDER BY ROFFSET DESC LIMIT 1")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nOffset)),
             false);

    if (query.next()) {
        result.bValid = true;
        result.nAddress = query.value(0).toULongLong();
        result.nOffset = query.value(1).toLongLong();
        result.nSize = query.value(2).toLongLong();
        result.recordType = (RT)query.value(3).toULongLong();
        result.nRefTo = query.value(4).toULongLong();
        result.nRefFrom = query.value(5).toULongLong();
        result.nBranch = query.value(6).toULongLong();
        result.dbstatus = (DBSTATUS)query.value(7).toLongLong();
    }
#else
    Q_UNUSED(nOffset)
#endif

    return result;
}

XInfoDB::SHOWRECORD XInfoDB::getShowRecordByOffset(qint64 nOffset)
{
    XInfoDB::SHOWRECORD result = {};

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE ROFFSET = '%2'")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nOffset)),
             false);

    if (query.next()) {
        result.nAddress = query.value(0).toULongLong();
        result.nOffset = query.value(1).toLongLong();
        result.nSize = query.value(2).toLongLong();
        result.recordType = (RT)query.value(3).toULongLong();
        result.nRefTo = query.value(4).toULongLong();
        result.nRefFrom = query.value(5).toULongLong();
        result.nBranch = query.value(6).toULongLong();
        result.dbstatus = (DBSTATUS)query.value(7).toLongLong();
    }
#else
    Q_UNUSED(nOffset)
#endif

    return result;
}

qint64 XInfoDB::getShowRecordOffsetByAddress(XADDR nAddress)
{
    qint64 nResult = 0;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT MIN(ROFFSET) FROM %1 WHERE ADDRESS >= %2").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)), false);

    if (query.next()) {
        nResult = query.value(0).toLongLong();
    }
#else
    Q_UNUSED(nAddress)
#endif

    return nResult;
}

qint64 XInfoDB::getShowRecordPrevOffsetByAddress(XADDR nAddress)
{
    qint64 nResult = 0;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT MAX(ROFFSET) FROM %1 WHERE ADDRESS <= %2").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)), false);

    if (query.next()) {
        nResult = query.value(0).toLongLong();
    }
#else
    Q_UNUSED(nAddress)
#endif

    return nResult;
}

qint64 XInfoDB::getShowRecordOffsetByLine(qint64 nNumber)
{
    return getShowRecordOffsetByAddress(getShowRecordAddressByLine(nNumber));
}

XADDR XInfoDB::getShowRecordAddressByOffset(qint64 nOffset)
{
    XADDR nResult = 0;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT ADDRESS FROM %1 WHERE ROFFSET >= %2 LIMIT 1").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nOffset)), false);

    if (query.next()) {
        nResult = query.value(0).toULongLong();
    }
#else
    Q_UNUSED(nOffset)
#endif

    return nResult;
}

XADDR XInfoDB::getShowRecordAddressByLine(qint64 nLine)
{
    XADDR nResult = 0;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT ADDRESS FROM %1 WHERE LINENUMBER = %2").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nLine)), false);

    if (query.next()) {
        nResult = query.value(0).toULongLong();
    }
#else
    Q_UNUSED(nLine)
#endif

    return nResult;
}

qint64 XInfoDB::getShowRecordsCount()
{
    qint64 nResult = 0;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT count(*) FROM %1").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]), false);

    if (query.next()) {
        nResult = query.value(0).toLongLong();
    }
#endif

    return nResult;
}

qint64 XInfoDB::getShowRecordLineByAddress(XADDR nAddress)
{
    qint64 nResult = 0;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query, QString("SELECT max(LINENUMBER) FROM %1 WHERE ADDRESS <= %2").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress)), false);

    if (query.next()) {
        nResult = query.value(0).toLongLong();
    }
#else
    Q_UNUSED(nAddress)
#endif

    return nResult;
}

qint64 XInfoDB::getShowRecordLineByOffset(qint64 nOffset)
{
    return getShowRecordLineByAddress(getShowRecordAddressByOffset(nOffset));
}

void XInfoDB::updateShowRecordLine(XADDR nAddress, qint64 nLine)
{
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("UPDATE %1 SET LINENUMBER = %2 WHERE ADDRESS = %3").arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nLine), QString::number(nAddress)),
             true);
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(nLine)
#endif
}

QList<XInfoDB::SHOWRECORD> XInfoDB::getShowRecords(qint64 nLine, qint32 nCount)
{
    QList<XInfoDB::SHOWRECORD> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS, ROFFSET, SIZE, RECTYPE, REFTO, REFFROM, BRANCH, DBSTATUS FROM %1 WHERE LINENUMBER >= %2  ORDER BY ADDRESS LIMIT %3")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nLine), QString::number(nCount)),
             false);

    while (query.next()) {
        SHOWRECORD record = {};
        record.bValid = true;
        record.nAddress = query.value(0).toULongLong();
        record.nOffset = query.value(1).toLongLong();
        record.nSize = query.value(2).toLongLong();
        record.recordType = (RT)query.value(3).toULongLong();
        record.nRefTo = query.value(4).toULongLong();
        record.nRefFrom = query.value(5).toULongLong();
        record.nBranch = query.value(6).toULongLong();
        record.dbstatus = (DBSTATUS)query.value(7).toLongLong();

        listResult.append(record);
    }
#else
    Q_UNUSED(nLine)
    Q_UNUSED(nCount)
#endif

    return listResult;
}

QList<XADDR> XInfoDB::getShowRecordRelAddresses(XCapstone::RELTYPE relType, DBSTATUS dbstatus)
{
    QList<XADDR> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL;

    if (relType == XCapstone::RELTYPE_ALL) {
        sSQL = QString("SELECT DISTINCT XREFTORELATIVE FROM %1 WHERE RELTYPE != %2").arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(XCapstone::RELTYPE_NONE));
    } else if (relType == XCapstone::RELTYPE_JMP) {
        sSQL = QString("SELECT DISTINCT XREFTORELATIVE FROM %1 WHERE RELTYPE IN(%2, %3, %4)")
                   .arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(XCapstone::RELTYPE_JMP), QString::number(XCapstone::RELTYPE_JMP_COND),
                        QString::number(XCapstone::RELTYPE_JMP_UNCOND));
    } else {
        sSQL = QString("SELECT DISTINCT XREFTORELATIVE FROM %1 WHERE RELTYPE = %2").arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(relType));
    }

    if (dbstatus != DBSTATUS_NONE) {
        sSQL += QString(" AND DBSTATUS = '%1'").arg(dbstatus);
    }

    querySQL(&query, sSQL, false);

    while (query.next()) {
        XADDR nAddress = query.value(0).toULongLong();

        listResult.append(nAddress);
    }
#else
    Q_UNUSED(relType)
    Q_UNUSED(dbstatus)
#endif
    return listResult;
}

QList<XBinary::ADDRESSSIZE> XInfoDB::getShowRecordMemoryVariables(DBSTATUS dbstatus)
{
    QList<XBinary::ADDRESSSIZE> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL = QString("SELECT XREFTOMEMORY, MAX(MEMORYSIZE) FROM %1 WHERE MEMTYPE <> 0").arg(s_sql_tableName[DBTABLE_RELATIVS]);

    if (dbstatus != DBSTATUS_NONE) {
        sSQL += QString(" AND DBSTATUS = '%1'").arg(dbstatus);
    }

    sSQL += "GROUP BY XREFTOMEMORY";

    querySQL(&query, sSQL, false);

    while (query.next()) {
        XBinary::ADDRESSSIZE record = {};
        record.nAddress = query.value(0).toULongLong();
        record.nSize = query.value(1).toLongLong();

        listResult.append(record);
    }
#else
    Q_UNUSED(dbstatus)
#endif
    return listResult;
}

QList<XBinary::ADDRESSSIZE> XInfoDB::getBranches(DBSTATUS dbstatus)
{
    QList<XBinary::ADDRESSSIZE> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL = QString("SELECT MIN(ADDRESS), MAX(ADDRESS+SIZE) FROM %1 WHERE BRANCH > 0").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]);

    if (dbstatus != DBSTATUS_NONE) {
        sSQL += QString(" AND DBSTATUS = '%1'").arg(dbstatus);
    }

    sSQL += "GROUP BY BRANCH ORDER BY ADDRESS";

    querySQL(&query, sSQL, false);

    while (query.next()) {
        XBinary::ADDRESSSIZE record = {};
        record.nAddress = query.value(0).toULongLong();
        record.nSize = query.value(1).toULongLong() - record.nAddress;

        listResult.append(record);
    }
#else
    Q_UNUSED(dbstatus)
#endif
    return listResult;
}

QList<XADDR> XInfoDB::getExportSymbolAddresses()
{
    QList<XADDR> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL = QString("SELECT DISTINCT ADDRESS FROM %1").arg(s_sql_tableName[DBTABLE_EXPORT]);

    querySQL(&query, sSQL, false);

    while (query.next()) {
        XADDR nAddress = query.value(0).toULongLong();

        listResult.append(nAddress);
    }
#endif
    return listResult;
}

QList<XADDR> XInfoDB::getImportSymbolAddresses()
{
    QList<XADDR> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL = QString("SELECT DISTINCT ADDRESS FROM %1").arg(s_sql_tableName[DBTABLE_IMPORT]);

    querySQL(&query, sSQL, false);

    while (query.next()) {
        XADDR nAddress = query.value(0).toULongLong();

        listResult.append(nAddress);
    }
#endif
    return listResult;
}

QList<XADDR> XInfoDB::getTLSSymbolAddresses()
{
    QList<XADDR> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL = QString("SELECT DISTINCT ADDRESS FROM %1").arg(s_sql_tableName[DBTABLE_TLS]);

    querySQL(&query, sSQL, false);

    while (query.next()) {
        XADDR nAddress = query.value(0).toULongLong();

        listResult.append(nAddress);
    }
#endif
    return listResult;
}

QList<XADDR> XInfoDB::getFunctionAddresses()
{
    QList<XADDR> listResult;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    QString sSQL = QString("SELECT DISTINCT ADDRESS FROM %1").arg(s_sql_tableName[DBTABLE_FUNCTIONS]);

    querySQL(&query, sSQL, false);

    while (query.next()) {
        XADDR nAddress = query.value(0).toULongLong();

        listResult.append(nAddress);
    }
#endif
    return listResult;
}

XInfoDB::RELRECORD XInfoDB::getRelRecordByAddress(XADDR nAddress)
{
    XInfoDB::RELRECORD result = {};

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);

    querySQL(&query,
             QString("SELECT ADDRESS, RELTYPE, XREFTORELATIVE, MEMTYPE, XREFTOMEMORY, MEMORYSIZE FROM %1 WHERE ADDRESS = %2")
                 .arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(nAddress)),
             false);

    if (query.next()) {
        result.nAddress = query.value(0).toULongLong();
        result.relType = (XCapstone::RELTYPE)query.value(1).toULongLong();  // TODO
        result.nXrefToRelative = query.value(2).toULongLong();
        result.memType = (XCapstone::MEMTYPE)query.value(3).toULongLong();  // TODO
        result.nXrefToMemory = query.value(4).toULongLong();
        result.nMemorySize = query.value(5).toLongLong();
    }
#else
    Q_UNUSED(nAddress)
#endif

    return result;
}

bool XInfoDB::isAddressHasRefFrom(XADDR nAddress)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);
    // mb TODO more checks
    querySQL(
        &query,
        QString("SELECT ADDRESS FROM %1 WHERE (XREFTORELATIVE = %2) OR (XREFTOMEMORY = %2) LIMIT 1").arg(s_sql_tableName[DBTABLE_RELATIVS], QString::number(nAddress)),
        false);

    if (query.next()) {
        bResult = true;
    }
#else
    Q_UNUSED(nAddress)
#endif

    return bResult;
}

bool XInfoDB::isAnalyzedRegionVirtual(XADDR nAddress, qint64 nSize)
{
    bool bResult = false;
#ifdef QT_SQL_LIB
    QSqlQuery query(g_dataBase);
    querySQL(&query,
             QString("SELECT ADDRESS FROM %1 WHERE ADDRESS >= %2 AND ADDRESS < %3 AND ROFFSET = -1")
                 .arg(s_sql_tableName[DBTABLE_SHOWRECORDS], QString::number(nAddress), QString::number(nAddress + nSize)),
             false);

    bResult = query.next();
#else
    Q_UNUSED(nAddress)
    Q_UNUSED(nSize)
#endif
    return bResult;
}

bool XInfoDB::loadDbFromFile(const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlDatabase dataBase = QSqlDatabase::addDatabase("QSQLITE", "local_db");
    dataBase.setDatabaseName(sDBFileName);

    if (dataBase.open()) {
        bResult = copyDb(&dataBase, &g_dataBase, pPdStruct);

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

bool XInfoDB::saveDbToFile(const QString &sDBFileName, XBinary::PDSTRUCT *pPdStruct)
{
    bool bResult = false;

#ifdef QT_SQL_LIB
    QSqlDatabase dataBase = QSqlDatabase::addDatabase("QSQLITE", "local_db");
    dataBase.setDatabaseName(sDBFileName);

    if (dataBase.open()) {
        bResult = copyDb(&g_dataBase, &dataBase, pPdStruct);

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

    if (bResult && bWrite) {
        setDatabaseChanged(true);
    }

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
#ifdef QT_SQL_LIB
bool XInfoDB::copyDb(QSqlDatabase *pDatabaseSource, QSqlDatabase *pDatabaseDest, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    // initDb(pDatabaseDest); // TODO replace to is Table exists ->

    QSqlQuery queryRead(*pDatabaseSource);
    QSqlQuery queryWrite(*pDatabaseDest);

    bool bResult = false;

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_BOOKMARKS
        if (isTablePresent(pDatabaseSource, DBTABLE_BOOKMARKS)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_BOOKMARKS);
            createTable(pDatabaseDest, DBTABLE_BOOKMARKS);

            querySQL(&queryRead,
                     QString("SELECT UUID, LOCATION, LOCTYPE, SIZE, COLTEXT, COLBACKGROUND, TEMPLATE, COMMENT FROM %1").arg(s_sql_tableName[DBTABLE_BOOKMARKS]), false);

            queryWrite.prepare(QString("INSERT INTO %1 (UUID, LOCATION, LOCTYPE, SIZE, COLTEXT, COLBACKGROUND, TEMPLATE, COMMENT) "
                                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_BOOKMARKS]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));
                queryWrite.bindValue(2, queryRead.value(2));
                queryWrite.bindValue(3, queryRead.value(3));
                queryWrite.bindValue(4, queryRead.value(4));
                queryWrite.bindValue(5, queryRead.value(5));
                queryWrite.bindValue(6, queryRead.value(6));
                queryWrite.bindValue(7, queryRead.value(7));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_COMMENTS
        if (isTablePresent(pDatabaseSource, DBTABLE_COMMENTS)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_COMMENTS);
            createTable(pDatabaseDest, DBTABLE_COMMENTS);

            querySQL(&queryRead, QString("SELECT LOCATION, COMMENT FROM %1").arg(s_sql_tableName[DBTABLE_COMMENTS]), false);

            queryWrite.prepare(QString("INSERT INTO %1 (LOCATION, COMMENT) "
                                       "VALUES (?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_COMMENTS]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_SYMBOLS
        if (isTablePresent(pDatabaseSource, DBTABLE_SYMBOLS)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_SYMBOLS);
            createTable(pDatabaseDest, DBTABLE_SYMBOLS);

            querySQL(&queryRead, QString("SELECT ADDRESS, MODULE, SYMTEXT FROM %1").arg(s_sql_tableName[DBTABLE_SYMBOLS]), false);

            queryWrite.prepare(QString("INSERT INTO %1 (ADDRESS, MODULE, SYMTEXT) "
                                       "VALUES (?, ?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_SYMBOLS]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));
                queryWrite.bindValue(2, queryRead.value(2));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_SHOWRECORDS
        if (isTablePresent(pDatabaseSource, DBTABLE_SHOWRECORDS)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_SHOWRECORDS);
            createTable(pDatabaseDest, DBTABLE_SHOWRECORDS);

            querySQL(&queryRead,
                     QString("SELECT ADDRESS, ROFFSET, SIZE, RECTEXT1, RECTEXT2, RECTYPE, LINENUMBER, REFTO, REFFROM FROM %1").arg(s_sql_tableName[DBTABLE_SHOWRECORDS]),
                     false);

            queryWrite.prepare(QString("INSERT INTO %1 (ADDRESS, ROFFSET, SIZE, RECTEXT1, RECTEXT2, RECTYPE, LINENUMBER, REFTO, REFFROM) "
                                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_SHOWRECORDS]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));
                queryWrite.bindValue(2, queryRead.value(2));
                queryWrite.bindValue(3, queryRead.value(3));
                queryWrite.bindValue(4, queryRead.value(4));
                queryWrite.bindValue(5, queryRead.value(5));
                queryWrite.bindValue(4, queryRead.value(6));
                queryWrite.bindValue(5, queryRead.value(7));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_RELATIVS
        if (isTablePresent(pDatabaseSource, DBTABLE_RELATIVS)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_RELATIVS);
            createTable(pDatabaseDest, DBTABLE_RELATIVS);

            querySQL(&queryRead, QString("SELECT ADDRESS, RELTYPE, XREFTORELATIVE, MEMTYPE, XREFTOMEMORY, MEMORYSIZE FROM %1").arg(s_sql_tableName[DBTABLE_RELATIVS]),
                     false);

            queryWrite.prepare(QString("INSERT INTO %1 (ADDRESS, RELTYPE, XREFTORELATIVE, MEMTYPE, XREFTOMEMORY, MEMORYSIZE) "
                                       "VALUES (?, ?, ?, ?, ?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_RELATIVS]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));
                queryWrite.bindValue(2, queryRead.value(2));
                queryWrite.bindValue(3, queryRead.value(3));
                queryWrite.bindValue(4, queryRead.value(4));
                queryWrite.bindValue(5, queryRead.value(5));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_IMPORT
        if (isTablePresent(pDatabaseSource, DBTABLE_IMPORT)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_IMPORT);
            createTable(pDatabaseDest, DBTABLE_IMPORT);

            querySQL(&queryRead, QString("SELECT ADDRESS, ORIGNAME FROM %1").arg(s_sql_tableName[DBTABLE_IMPORT]), false);

            queryWrite.prepare(QString("INSERT INTO %1 (ADDRESS, ORIGNAME) "
                                       "VALUES (?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_IMPORT]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_EXPORT
        if (isTablePresent(pDatabaseSource, DBTABLE_EXPORT)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_EXPORT);
            createTable(pDatabaseDest, DBTABLE_EXPORT);

            querySQL(&queryRead, QString("SELECT ADDRESS, ORIGNAME FROM %1").arg(s_sql_tableName[DBTABLE_EXPORT]), false);

            queryWrite.prepare(QString("INSERT INTO %1 (ADDRESS, ORIGNAME) "
                                       "VALUES (?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_EXPORT]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_TLS
        if (isTablePresent(pDatabaseSource, DBTABLE_TLS)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_TLS);
            createTable(pDatabaseDest, DBTABLE_TLS);

            querySQL(&queryRead, QString("SELECT ADDRESS, ORIGNAME FROM %1").arg(s_sql_tableName[DBTABLE_TLS]), false);

            queryWrite.prepare(QString("INSERT INTO %1 (ADDRESS, ORIGNAME) "
                                       "VALUES (?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_TLS]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        // DBTABLE_FUNCTIONS
        if (isTablePresent(pDatabaseSource, DBTABLE_FUNCTIONS)) {
            pDatabaseDest->transaction();
            removeTable(pDatabaseDest, DBTABLE_FUNCTIONS);
            createTable(pDatabaseDest, DBTABLE_FUNCTIONS);

            querySQL(&queryRead, QString("SELECT ADDRESS, SIZE, NAME FROM %1").arg(s_sql_tableName[DBTABLE_FUNCTIONS]), false);

            queryWrite.prepare(QString("INSERT INTO %1 (ADDRESS, SIZE, NAME) "
                                       "VALUES (?, ?, ?)")
                                   .arg(s_sql_tableName[DBTABLE_FUNCTIONS]));

            while (queryRead.next() && (!(pPdStruct->bIsStop))) {
                queryWrite.bindValue(0, queryRead.value(0));
                queryWrite.bindValue(1, queryRead.value(1));
                queryWrite.bindValue(2, queryRead.value(2));

                querySQL(&queryWrite, true);
            }

            pDatabaseDest->commit();
        }
    }

    if (!(pPdStruct->bIsStop)) {
        bResult = querySQL(&queryWrite, QString("VACUUM"), false);
    }

    return bResult;
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
    g_bIsDebugger = bState;
    initDB();
}

bool XInfoDB::isDebugger()
{
    return g_bIsDebugger;
}
#ifdef QT_GUI_LIB
QColor XInfoDB::stringToColor(const QString &sCode)
{
    QColor color;
    color.setNamedColor(sCode);

    return color;
}
#endif
#ifdef QT_GUI_LIB
QString XInfoDB::colorToString(QColor color)
{
    return color.name();
}
#endif
QString XInfoDB::convertOpcodeString(XCapstone::DISASM_RESULT disasmResult, XBinary::SYNTAX syntax, const XInfoDB::RI_TYPE &riType, const XCapstone::DISASM_OPTIONS &disasmOptions)
{
    QString sResult = disasmResult.sString;

    if (disasmResult.relType) {
        sResult = _convertOpcodeString(sResult, disasmResult.nXrefToRelative, syntax, riType, disasmOptions);
    }

    if (disasmResult.memType) {
        sResult = _convertOpcodeString(sResult, disasmResult.nXrefToMemory, syntax, riType, disasmOptions);
    }

    return sResult;
}

QString XInfoDB::_convertOpcodeString(QString sString, XADDR nAddress, XBinary::SYNTAX syntax, const RI_TYPE &riType, const XCapstone::DISASM_OPTIONS &disasmOptions)
{
    QString sResult = sString;

    QString sReplace = XInfoDB::recordInfoToString(getRecordInfoCache(nAddress), riType);
    QString sOrigin;

    if ((syntax == XBinary::SYNTAX_DEFAULT) || (syntax == XBinary::SYNTAX_INTEL)) {
        sOrigin = QString("0x%1").arg(QString::number(nAddress, 16));
    } else if (syntax == XBinary::SYNTAX_MASM) {
        sOrigin = QString("%1h").arg(QString::number(nAddress, 16));
    } else if (syntax == XBinary::SYNTAX_ATT) {
        sOrigin = QString("0x%1").arg(QString::number(nAddress, 16));
    }

    if (disasmOptions.bIsUppercase) {
        sOrigin = sOrigin.toUpper();
    }

    sResult = sResult.replace(sOrigin, sReplace);

    return sResult;
}

void XInfoDB::setDatabaseChanged(bool bState)
{
    g_bIsDatabaseChanged = bState;
}

bool XInfoDB::isDatabaseChanged()
{
    return g_bIsDatabaseChanged;
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

    for (int i = 0; i < nNumberOfRecords; i++) {
        if (pListRegs->at(i).reg == _reg) {
            result = pListRegs->at(i).value;
            break;
        }
    }

    if (result.mode != XBinary::MODE_UNKNOWN) {
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

    for (int i = 0; i < nNumberOfRecords; i++) {
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

    if (variant.mode == XBinary::MODE_32) {
#ifdef Q_PROCESSOR_X86_32
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::getBitFromDword(result.var.v_uint32, RFLAGS_BIT_OF));
#endif
    } else if (variant.mode == XBinary::MODE_64) {
#ifdef Q_PROCESSOR_X86_64
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::getBitFromQword(result.var.v_uint64, RFLAGS_BIT_OF));
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

    if (variant.mode == XBinary::MODE_32) {
#ifdef Q_PROCESSOR_X86_32
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::setBitToDword(result.var.v_uint32, bValue, RFLAGS_BIT_OF));
#endif
    } else if (variant.mode == XBinary::MODE_64) {
#ifdef Q_PROCESSOR_X86_64
        // TODO Check mb reg32
        if (reg == XREG_FLAGS_CF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_CF));
        else if (reg == XREG_FLAGS_PF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_PF));
        else if (reg == XREG_FLAGS_AF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_AF));
        else if (reg == XREG_FLAGS_ZF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_ZF));
        else if (reg == XREG_FLAGS_SF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_SF));
        else if (reg == XREG_FLAGS_TF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_TF));
        else if (reg == XREG_FLAGS_IF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_IF));
        else if (reg == XREG_FLAGS_DF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_DF));
        else if (reg == XREG_FLAGS_OF) result = XBinary::getXVariant(XBinary::setBitToQword(result.var.v_uint64, bValue, RFLAGS_BIT_OF));
#endif
    }

    return result;
}
#endif
