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
#include "dialogxinfodbtransferprocess.h"

DialogXInfoDBTransferProcess::DialogXInfoDBTransferProcess(QWidget *pParent) : XDialogProcess(pParent)
{
    g_pTransfer = new XInfoDBTransfer;
    g_pThread = new QThread;

    g_pTransfer->moveToThread(g_pThread);

    connect(g_pThread, SIGNAL(started()), g_pTransfer, SLOT(process()));
    connect(g_pTransfer, SIGNAL(completed(qint64)), this, SLOT(onCompleted(qint64)));
    connect(g_pTransfer, SIGNAL(errorMessage(QString)), this, SLOT(errorMessage(QString)));
    //    connect(g_pTransfer,SIGNAL(progressValueChanged(qint32)),this,SLOT(onProgressValueChanged(qint32)));
}

DialogXInfoDBTransferProcess::~DialogXInfoDBTransferProcess()
{
    g_pThread->quit();
    g_pThread->wait();

    delete g_pThread;
    delete g_pTransfer;
}

void DialogXInfoDBTransferProcess::analyze(XInfoDB *pXInfoDB, QString sFileName, XBinary::FT fileType)
{
    setWindowTitle(tr("Analyze"));

    g_pTransfer->setData(pXInfoDB, XInfoDBTransfer::TT_ANALYZE, sFileName, fileType, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::analyze(XInfoDB *pXInfoDB, QIODevice *pDevice, XBinary::FT fileType)
{
    // TODO if FT_UNKNOWN show a dialog with options
    setWindowTitle(tr("Analyze"));

    g_pTransfer->setData(pXInfoDB, XInfoDBTransfer::TT_ANALYZE, pDevice, fileType, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::symbols(XInfoDB *pXInfoDB, QString sFileName, XBinary::FT fileType)
{
    setWindowTitle(tr("Symbols"));

    g_pTransfer->setData(pXInfoDB, XInfoDBTransfer::TT_SYMBOLS, sFileName, fileType, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::symbols(XInfoDB *pXInfoDB, QIODevice *pDevice, XBinary::FT fileType)
{
    setWindowTitle(tr("Symbols"));

    g_pTransfer->setData(pXInfoDB, XInfoDBTransfer::TT_SYMBOLS, pDevice, fileType, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::clear(XInfoDB *pXInfoDB)
{
    setWindowTitle(tr("Clear"));

    g_pTransfer->setData(pXInfoDB, XInfoDBTransfer::TT_CLEAR, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::exportData(XInfoDB *pXInfoDB, QString sFileName)
{
    setWindowTitle(tr("Export"));

    g_pTransfer->setData(pXInfoDB, XInfoDBTransfer::TT_EXPORT, sFileName, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::importData(XInfoDB *pXInfoDB, QString sFileName)
{
    setWindowTitle(tr("Import"));

    g_pTransfer->setData(pXInfoDB, XInfoDBTransfer::TT_IMPORT, sFileName, getPdStruct());
    g_pThread->start();
}
