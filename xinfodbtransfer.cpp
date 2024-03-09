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
                        analyzeOptions.bIsInit = true;
                        analyzeOptions.nStartAddress = -1;
                    } else if (g_transferType == COMMAND_ANALYZE) {
                        analyzeOptions.bIsInit = false;
                        analyzeOptions.nStartAddress = g_options.nAddress;
                    } else if (g_transferType == COMMAND_DISASM) {
                        analyzeOptions.bIsInit = false;
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
            // TODO unset dtatabase changed
        }
    }

    if (g_transferType == COMMAND_SCANFORIAT) {
                // TODO
    } else if (g_transferType == COMMAND_GETIAT) {
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
        QMap<XADDR, QString> mapFunctions;

        QList<XProcess::MODULE> listModules = XProcess::getModulesList(g_options.nProcessID, g_pPdStruct);

        {
            qint32 _nFreeIndexScan = XBinary::getFreeIndex(g_pPdStruct);
            qint32 nNumberOfModules = listModules.count();

            XBinary::setPdStructTotal(g_pPdStruct, _nFreeIndexScan, nNumberOfModules);

            for (qint32 i = 0; (i < nNumberOfModules) && (!(g_pPdStruct->bIsStop)); i++) {
                XBinary::setPdStructStatus(g_pPdStruct, _nFreeIndexScan, listModules.at(i).sName);

                XProcess xprocess(g_options.nProcessID, listModules.at(i).nAddress, listModules.at(i).nSize);

                if (xprocess.open(QIODevice::ReadOnly)) {
                    XPE pe(&xprocess, true, listModules.at(i).nAddress);

                    if (pe.isValid(g_pPdStruct)) {
                        XPE::EXPORT_HEADER exportHeader = pe.getExport(false, g_pPdStruct);

                        qint32 nNumberOfPositions = exportHeader.listPositions.count();

                        for (qint32 j = 0; j < nNumberOfPositions; j++) {
                            QString sFunctionName = exportHeader.listPositions.at(j).sFunctionName;
                            if (sFunctionName == "") {
                                sFunctionName = QString::number(exportHeader.listPositions.at(j).nOrdinal);
                            }

                            mapFunctions.insert(exportHeader.listPositions.at(j).nAddress, exportHeader.sName + "#" + sFunctionName);
                        }
                    }

                    xprocess.close();
                }

                XBinary::setPdStructCurrent(g_pPdStruct, _nFreeIndexScan, i);
            }

            XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndexScan);
        }


        // TODO
        XADDR nStartAddress = g_options.nAddress;
        qint64 nSize = g_options.nSize;

#ifndef Q_OS_WIN64
        // bool bIs64 = false;
        qint32 nStep = 4;
#else
        // bool bIs64 = true;
        qint32 nStep = 8;
#endif
        qint32 _nFreeIndexScan = XBinary::getFreeIndex(g_pPdStruct);
        XBinary::setPdStructInit(g_pPdStruct, _nFreeIndexScan, nSize/nStep);

        // TODO

        XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndexScan);
#endif
#endif
    }

    XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndex);

    emit completed(scanTimer.elapsed());

    return bResult;
}
