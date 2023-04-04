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
    g_transferType = TT_ANALYZE;
    g_fileType = XBinary::FT_UNKNOWN;
    g_pDevice = nullptr;
    g_pPdStruct = nullptr;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB, TT transferType, QString sFileName, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct)
{
    g_pXInfoDB = pXInfoDB;
    g_transferType = transferType;
    g_sFileName = sFileName;
    g_fileType = fileType;
    g_pPdStruct = pPdStruct;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB, TT transferType, QIODevice *pDevice, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct)
{
    g_pXInfoDB = pXInfoDB;
    g_transferType = transferType;
    g_pDevice = pDevice;
    g_fileType = fileType;
    g_pPdStruct = pPdStruct;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB, TT transferType, QString sFileName, XBinary::PDSTRUCT *pPdStruct)
{
    g_pXInfoDB = pXInfoDB;
    g_transferType = transferType;
    g_sFileName = sFileName;
    g_pPdStruct = pPdStruct;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB, TT transferType, XBinary::PDSTRUCT *pPdStruct)
{
    g_pXInfoDB = pXInfoDB;
    g_transferType = transferType;
    g_pPdStruct = pPdStruct;
}

bool XInfoDBTransfer::process()
{
    // TODO get string are not in code
    bool bResult = false;

    QElapsedTimer scanTimer;
    scanTimer.start();

    qint32 _nFreeIndex = XBinary::getFreeIndex(g_pPdStruct);
    XBinary::setPdStructInit(g_pPdStruct, _nFreeIndex, 0);

    if (g_pXInfoDB) {
        if (g_transferType == TT_ANALYZE) {
            QIODevice *pDevice = g_pDevice;

            bool bFile = false;

            if ((!pDevice) && (g_sFileName != "")) {
                bFile = true;

                QFile *pFile = new QFile;

                pFile->setFileName(g_sFileName);

                if (pFile->open(QIODevice::ReadOnly)) {
                    pDevice = pFile;
                } else {
                    delete pFile;
                }
            }

            if (pDevice) {
                g_pXInfoDB->initDb();  // TODO Check

                g_pXInfoDB->_addSymbols(pDevice, g_fileType, g_pPdStruct);

                XBinary::_MEMORY_MAP memoryMap = XFormats::getMemoryMap(g_fileType, pDevice);

                g_pXInfoDB->_disasmAnalyze(pDevice, &memoryMap, memoryMap.nEntryPointAddress, true, g_pPdStruct);
                // TODO sort records
            }

            g_pXInfoDB->setAnalyzed(g_pXInfoDB->isSymbolsPresent());

            if (bFile && pDevice) {
                QFile *pFile = static_cast<QFile *>(pDevice);

                pFile->close();

                delete pFile;
            }
        } else if (g_transferType == TT_EXPORT) {
            // TODO
        } else if (g_transferType == TT_CLEAR) {
            g_pXInfoDB->clearDb();
            g_pXInfoDB->setAnalyzed(g_pXInfoDB->isSymbolsPresent());
        }
    }

    XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndex);

    emit completed(scanTimer.elapsed());

    return bResult;
}
