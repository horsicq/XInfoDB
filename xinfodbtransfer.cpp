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
        if ((g_transferType == COMMAND_ANALYZEALL) || (g_transferType == COMMAND_ANALYZE)) {
            bool bFile = false;
            QIODevice *pDevice = g_options.pDevice;

            if ((!g_options.pDevice) && (g_options.sFileName != "")) {
                bFile = true;

                QFile *pFile = new QFile;

                pFile->setFileName(g_options.sFileName);

                if (pFile->open(QIODevice::ReadOnly)) {
                    pDevice = pFile;
                } else {
                    delete pFile;
                }
            }

            if ((g_transferType == COMMAND_ANALYZEALL) || (g_transferType == COMMAND_ANALYZE)) {
                if (pDevice) {
                    g_pXInfoDB->addMode(g_options.pDevice, g_options.fileType);
                    g_pXInfoDB->_analyze(g_options.fileType, g_pPdStruct);
                }
            }

            if (bFile && pDevice) {
                QFile *pFile = static_cast<QFile *>(pDevice);

                pFile->close();

                delete pFile;
            }
        } else if (g_transferType == COMMAND_IMPORT) {
            g_pXInfoDB->saveDbToFile(g_options.sFileName, g_pPdStruct);
        } else if (g_transferType == COMMAND_EXPORT) {
            // g_pXInfoDB->loadDbFromFile(g_options.sFileName, g_pPdStruct);
        }
    }

    XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndex);

    emit completed(scanTimer.elapsed());

    return bResult;
}
