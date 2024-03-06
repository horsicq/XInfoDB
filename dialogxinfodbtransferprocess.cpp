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
    connect(g_pTransfer, SIGNAL(errorMessage(QString)), this, SLOT(errorMessageSlot(QString)));
}

DialogXInfoDBTransferProcess::~DialogXInfoDBTransferProcess()
{
    g_pThread->quit();
    g_pThread->wait();

    delete g_pThread;
    delete g_pTransfer;
}

void DialogXInfoDBTransferProcess::setData(XInfoDB *pXInfoDB, XInfoDBTransfer::COMMAND command, const XInfoDBTransfer::OPTIONS &options)
{
    updateTitle(command);

    g_pTransfer->setData(pXInfoDB, command, options, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::setData(XInfoDBTransfer::COMMAND command, const XInfoDBTransfer::OPTIONS &options, XInfoDBTransfer::RESULT *pResult, XBinary::PDSTRUCT *pPdStruct)
{
    updateTitle(command);

    g_pTransfer->setData(command, options, pResult, getPdStruct());
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::saveDatabase(XInfoDB *pXInfoDB)
{
    if (pXInfoDB) {
        // TODO
    }
}

void DialogXInfoDBTransferProcess::updateTitle(XInfoDBTransfer::COMMAND command)
{
    QString sTitle;

    if ((command == XInfoDBTransfer::COMMAND_ANALYZEALL) || (command == XInfoDBTransfer::COMMAND_ANALYZE)) {
        // TODO if FT_UNKNOWN show a dialog with options
        sTitle = tr("Analyze");
    } else if (command == XInfoDBTransfer::COMMAND_DISASM) {
        sTitle = tr("Disasm");
    } else if (command == XInfoDBTransfer::COMMAND_SYMBOLS) {
        sTitle = tr("Symbols");
    } else if (command == XInfoDBTransfer::COMMAND_CLEAR) {
        sTitle = tr("Clear");
    } else if (command == XInfoDBTransfer::COMMAND_REMOVE) {
        sTitle = tr("Remove");
    } else if (command == XInfoDBTransfer::COMMAND_EXPORT) {
        sTitle = tr("Export");
    } else if (command == XInfoDBTransfer::COMMAND_IMPORT) {
        sTitle = tr("Import");
    } else if (command == XInfoDBTransfer::COMMAND_SCANFORIAT) {
        sTitle = tr("Scan for IAT");
    }

    setWindowTitle(sTitle);
}
