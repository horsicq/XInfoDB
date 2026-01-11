/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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

XInfoDBTransfer::XInfoDBTransfer(QObject *pParent) : XThreadObject(pParent)
{
    m_pXInfoDB = nullptr;
    m_transferType = COMMAND_ANALYZEALL;
    m_options = {};
    m_pResult = nullptr;
    m_pPdStruct = nullptr;
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    m_pListImports = nullptr;
#endif
#endif
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB, COMMAND transferType, const OPTIONS &options, XBinary::PDSTRUCT *pPdStruct)
{
    m_pXInfoDB = pXInfoDB;
    m_transferType = transferType;
    m_options = options;
    m_pPdStruct = pPdStruct;
}

void XInfoDBTransfer::setData(COMMAND transferType, const OPTIONS &options, RESULT *pResult, XBinary::PDSTRUCT *pPdStruct)
{
    m_transferType = transferType;
    m_options = options;
    m_pResult = pResult;
    m_pPdStruct = pPdStruct;
}
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
void XInfoDBTransfer::setData(COMMAND transferType, const OPTIONS &options, QList<XPE::IMPORT_RECORD> *pListImports, XBinary::PDSTRUCT *pPdStruct)
{
    m_transferType = transferType;
    m_options = options;
    m_pListImports = pListImports;
    m_pPdStruct = pPdStruct;
}
#endif
#endif
void XInfoDBTransfer::process()
{
#ifdef QT_DEBUG
    qDebug("bool XInfoDBTransfer::process()");
#endif
    // TODO get string are not in code

    qint32 _nFreeIndex = XBinary::getFreeIndex(m_pPdStruct);
    XBinary::setPdStructInit(m_pPdStruct, _nFreeIndex, 0);

    if (m_pXInfoDB) {
        if ((m_transferType == COMMAND_ANALYZEALL) || (m_transferType == COMMAND_ANALYZE)) {
            bool bFile = false;
            QIODevice *pDevice = m_options.pDevice;

            if ((!m_options.pDevice) && (m_options.sDatabaseFileName != "")) {
                bFile = true;

                QFile *pFile = new QFile;

                pFile->setFileName(m_options.sDatabaseFileName);

                if (pFile->open(QIODevice::ReadOnly)) {
                    pDevice = pFile;
                } else {
                    delete pFile;
                }
            }

            if ((m_transferType == COMMAND_ANALYZEALL) || (m_transferType == COMMAND_ANALYZE)) {
                if (pDevice) {
                    m_pXInfoDB->addMode(m_options.pDevice, m_options.fileType);
                    m_pXInfoDB->_analyze(m_options.fileType, m_pPdStruct);
                }
            }

            if (bFile && pDevice) {
                QFile *pFile = static_cast<QFile *>(pDevice);

                pFile->close();

                delete pFile;
            }
        } else if (m_transferType == COMMAND_IMPORT) {
            m_pXInfoDB->saveDbToFile(m_options.sDatabaseFileName, m_pPdStruct);
        } else if (m_transferType == COMMAND_EXPORT) {
            m_pXInfoDB->loadDbFromFile(m_options.pDevice, m_options.sDatabaseFileName, m_pPdStruct);
        }
    }

    XBinary::setPdStructFinished(m_pPdStruct, _nFreeIndex);
}
