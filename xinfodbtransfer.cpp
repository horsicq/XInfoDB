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
#include "xinfodbtransfer.h"

XInfoDBTransfer::XInfoDBTransfer(QObject *pParent) : QObject(pParent)
{
    g_pXInfoDB = nullptr;
    g_transferType = COMMAND_ANALYZEALL;
    g_options = {};
    g_pResult = nullptr;
    g_pPdStruct = nullptr;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    g_pListImports = nullptr;
#endif
#endif
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB, COMMAND transferType, const OPTIONS &options, XBinary::PDSTRUCT *pPdStruct)
{
    g_pXInfoDB = pXInfoDB;
    g_transferType = transferType;
    g_options = options;
    g_pPdStruct = pPdStruct;
}

void XInfoDBTransfer::setData(COMMAND transferType, const OPTIONS &options, RESULT *pResult, XBinary::PDSTRUCT *pPdStruct)
{
    g_transferType = transferType;
    g_options = options;
    g_pResult = pResult;
    g_pPdStruct = pPdStruct;
}
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
void XInfoDBTransfer::setData(COMMAND transferType, const OPTIONS &options, QList<XPE::IMPORT_RECORD> *pListImports, XBinary::PDSTRUCT *pPdStruct)
{
    g_transferType = transferType;
    g_options = options;
    g_pListImports = pListImports;
    g_pPdStruct = pPdStruct;
}
#endif
#endif
bool XInfoDBTransfer::process()
{
#ifdef QT_DEBUG
    qDebug("bool XInfoDBTransfer::process()");
#endif
    // TODO get string are not in code
    bool bResult = false;

    QElapsedTimer scanTimer;
    scanTimer.start();

    qint32 _nFreeIndex = XBinary::getFreeIndex(g_pPdStruct);
    XBinary::setPdStructInit(g_pPdStruct, _nFreeIndex, 0);

    if (g_pXInfoDB) {
        if ((g_transferType == COMMAND_ANALYZEALL) || (g_transferType == COMMAND_ANALYZE) || (g_transferType == COMMAND_SYMBOLS) || (g_transferType == COMMAND_DISASM)) {
            QIODevice *pDevice = g_options.pDevice;

            bool bFile = false;

            if ((!pDevice) && (g_options.sFileName != "")) {
                bFile = true;

                QFile *pFile = new QFile;

                pFile->setFileName(g_options.sFileName);

                if (pFile->open(QIODevice::ReadOnly)) {
                    pDevice = pFile;
                } else {
                    delete pFile;
                }
            }

            if ((g_transferType == COMMAND_ANALYZEALL) || (g_transferType == COMMAND_ANALYZE) || (g_transferType == COMMAND_DISASM)) {
                if (pDevice) {
                    g_pXInfoDB->clearRecordInfoCache();

                    if ((!(g_pXInfoDB->isSymbolsPresent())) || (g_transferType == COMMAND_ANALYZEALL)) {
                        g_pXInfoDB->_addSymbolsFromFile(pDevice, g_options.bIsImage, g_options.nModuleAddress, g_options.fileType, g_pPdStruct);
                    }

                    g_pXInfoDB->initDisasmDb();

                    XBinary::_MEMORY_MAP memoryMap = XFormats::getMemoryMap(g_options.fileType, XBinary::MAPMODE_UNKNOWN, pDevice);

                    XInfoDB::ANALYZEOPTIONS analyzeOptions = {};

                    if (g_transferType == COMMAND_ANALYZEALL) {
                        analyzeOptions.bAll = true;
                        analyzeOptions.nStartAddress = -1;
                    } else if (g_transferType == COMMAND_ANALYZE) {
                        analyzeOptions.bAll = false;
                        analyzeOptions.nStartAddress = g_options.nAddress;
                    } else if (g_transferType == COMMAND_DISASM) {
                        analyzeOptions.bAll = false;
                        analyzeOptions.nStartAddress = g_options.nAddress;
                        analyzeOptions.nCount = 1;
                    }

                    analyzeOptions.pDevice = pDevice;
                    analyzeOptions.pMemoryMap = &memoryMap;

                    bool bSuccess = g_pXInfoDB->_analyzeCode(analyzeOptions, g_pPdStruct);

                    if (bSuccess && (g_transferType == COMMAND_ANALYZEALL)) {
                        // TODO set analyze
                    }
                    // TODO sort records
                }
            } else if (g_transferType == COMMAND_SYMBOLS) {
                if (pDevice) {
                    //                    g_pXInfoDB->clearDb();
                    g_pXInfoDB->_addSymbolsFromFile(pDevice, g_options.bIsImage, g_options.nModuleAddress, g_options.fileType, g_pPdStruct);
                }
            }

            if (bFile && pDevice) {
                QFile *pFile = static_cast<QFile *>(pDevice);

                pFile->close();

                delete pFile;
            }
        } else if (g_transferType == COMMAND_EXPORT) {
            g_pXInfoDB->saveDbToFile(g_options.sFileName, g_pPdStruct);
        } else if (g_transferType == COMMAND_IMPORT) {
            g_pXInfoDB->loadDbFromFile(g_options.sFileName, g_pPdStruct);
        } else if (g_transferType == COMMAND_REMOVE) {
            g_pXInfoDB->_removeAnalyze(g_options.nAddress, g_options.nSize);
        } else if (g_transferType == COMMAND_CLEAR) {
            g_pXInfoDB->_clearAnalyze();
            // TODO unset analyze all flag
            // TODO unset datatabase changed
        }
    }

    if (g_transferType == COMMAND_SCANFORIAT) {
        // TODO
    } else if (g_transferType == COMMAND_GETIAT) {
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
        // QMap<XADDR, QString> mapFunctions;
        QList<XInfoDB::IAT_RECORD> listIATrecords;
        QSet<QString> stModules;

        QList<XProcess::MODULE> listModules = XProcess::getModulesList(g_options.nProcessID, g_pPdStruct);
        qint32 nNumberOfModules = listModules.count();

        {
#ifndef Q_OS_WIN64
            // bool bIs64 = false;
            qint32 nStep = 4;
#else
            // bool bIs64 = true;
            qint32 nStep = 8;
#endif
            qint32 _nFreeIndexScan = XBinary::getFreeIndex(g_pPdStruct);
            XBinary::setPdStructInit(g_pPdStruct, _nFreeIndexScan, g_options.nSize);

            XADDR nRegionAddress = S_ALIGN_DOWN64(g_options.nAddress, nStep);
            qint64 nRegionSize = S_ALIGN_UP64(g_options.nSize, nStep);

            XProcess xprocess(g_options.nProcessID, nRegionAddress, nRegionSize);

            if (xprocess.open(QIODevice::ReadOnly)) {
                XBinary binary(&xprocess, true, nRegionAddress);

                for (qint64 i = 0; (i < nRegionSize) && (!(g_pPdStruct->bIsStop)); i += nStep) {
                    XADDR nValue = 0;
                    if (nStep == 4) {
                        nValue = binary.read_uint32(i);
                    } else {
                        nValue = binary.read_uint64(i);
                    }

                    if (nValue) {
                        for (qint32 j = 0; (j < nNumberOfModules) && (!(g_pPdStruct->bIsStop)); j++) {
                            if ((nValue >= listModules.at(j).nAddress) && (nValue < listModules.at(j).nAddress + listModules.at(j).nSize)) {
                                XInfoDB::IAT_RECORD iatRecord = {};
                                iatRecord.nAddress = nRegionAddress + i;
                                iatRecord.nValue = nValue;

                                listIATrecords.append(iatRecord);
                                stModules.insert(listModules.at(j).sName);
                            }
                        }
                    }

                    XBinary::setPdStructCurrent(g_pPdStruct, _nFreeIndexScan, i);
                }

                xprocess.close();
            }

            XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndexScan);
        }

        qint32 nNumberOfIAT = listIATrecords.count();

        {
            qint32 _nFreeIndexScan = XBinary::getFreeIndex(g_pPdStruct);

            XBinary::setPdStructInit(g_pPdStruct, _nFreeIndexScan, nNumberOfModules);

            for (qint32 i = 0; (i < nNumberOfModules) && (!(g_pPdStruct->bIsStop)); i++) {
                QString sLibraryName = listModules.at(i).sName;
                XBinary::setPdStructStatus(g_pPdStruct, _nFreeIndexScan, sLibraryName);

                // TODO option deep scan
                if (stModules.contains(sLibraryName)) {
                    XADDR _nAddress = listModules.at(i).nAddress;
                    qint64 _nSize = listModules.at(i).nSize * 2;  // TODO fix kernel32
                    XProcess xprocess(g_options.nProcessID, _nAddress, _nSize);

                    if (xprocess.open(QIODevice::ReadOnly)) {
                        XPE pe(&xprocess, true, listModules.at(i).nAddress);

                        if (pe.isValid(g_pPdStruct)) {
                            XBinary::_MEMORY_MAP memoryMap = pe.getMemoryMap(XBinary::MAPMODE_UNKNOWN, g_pPdStruct);
                            XPE_DEF::IMAGE_EXPORT_DIRECTORY ied = pe.getExportDirectory();

                            QList<XADDR> listAddresses = pe.getExportFunctionAddressesList(&memoryMap, &ied, g_pPdStruct);
                            QList<quint16> listNameOrdinals = pe.getExportNameOrdinalsList(&memoryMap, &ied, g_pPdStruct);
                            QList<XADDR> listNames = pe.getExportNamesList(&memoryMap, &ied, g_pPdStruct);

                            for (qint32 j = 0; (j < nNumberOfIAT) && (!(g_pPdStruct->bIsStop)); j++) {
                                qint32 nOrdinal = listAddresses.indexOf(listIATrecords.at(j).nValue);

                                if (nOrdinal != -1) {
                                    qint32 nNameIndex = listNameOrdinals.indexOf(nOrdinal);

                                    QString sFunction;

                                    if (nNameIndex != -1) {
                                        if (nNameIndex < listNames.count()) {
                                            sFunction = pe.read_ansiString(listNames.at(nNameIndex) - memoryMap.nModuleAddress);
                                        }
                                    } else {
                                        sFunction = QString::number(nOrdinal + ied.Base);
                                    }

                                    listIATrecords[j].sFunction = QString("%1#%2").arg(sLibraryName, sFunction);
                                }
                            }
#ifdef QT_DEBUG
                            qDebug("%s", sLibraryName.toLatin1().data());
                            qint32 nNumberOfNames = listNames.count();

                            for (qint32 k = 0; k < nNumberOfNames; k++) {
                                quint16 nOrdinal = listNameOrdinals.at(k);
                                XADDR nAddress = listAddresses.at(nOrdinal);

                                QString sFunction = pe.read_ansiString(listNames.at(k) - memoryMap.nModuleAddress);

                                quint32 nForwardRVA = nAddress - memoryMap.nModuleAddress;

                                XPE_DEF::IMAGE_DATA_DIRECTORY idd = pe.getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

                                QString sForward;
                                if ((idd.VirtualAddress <= nForwardRVA) && (nForwardRVA < (idd.VirtualAddress + idd.Size))) {
                                    sForward = pe.read_ansiString(nForwardRVA);
                                }

                                qDebug("%llX %s -> %s ", nAddress, sFunction.toLatin1().data(), sForward.toLatin1().data());
                            }
#endif
                        }

                        xprocess.close();
                    }
                }

                XBinary::setPdStructCurrent(g_pPdStruct, _nFreeIndexScan, i);
            }

            XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndexScan);
        }
#endif
#endif
    }

    XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndex);

    emit completed(scanTimer.elapsed());

    return bResult;
}
