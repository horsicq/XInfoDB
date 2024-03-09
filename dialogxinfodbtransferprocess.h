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
#ifndef DIALOGXINFODBTRANSFERPROCESS_H
#define DIALOGXINFODBTRANSFERPROCESS_H

#include <QDialog>
#include <QThread>

#include "xdialogprocess.h"
#include "xinfodbtransfer.h"

class DialogXInfoDBTransferProcess : public XDialogProcess {
    Q_OBJECT

public:
    explicit DialogXInfoDBTransferProcess(QWidget *pParent = nullptr);
    ~DialogXInfoDBTransferProcess();

    void setData(XInfoDB *pXInfoDB, XInfoDBTransfer::COMMAND command, const XInfoDBTransfer::OPTIONS &options);
    void setData(XInfoDBTransfer::COMMAND command, const XInfoDBTransfer::OPTIONS &options, XInfoDBTransfer::RESULT *pResult);
#ifdef USE_XPROCESS
#ifdef Q_OS_WIN
    void setData(XInfoDBTransfer::COMMAND command, const XInfoDBTransfer::OPTIONS &options, QList<XPE::IMPORT_RECORD> *pListImports);
#endif
#endif
    static void saveDatabase(XInfoDB *pXInfoDB);

private:
    void updateTitle(XInfoDBTransfer::COMMAND command);

private:
    XInfoDBTransfer *g_pTransfer;
    QThread *g_pThread;
};

#endif  // DIALOGXINFODBTRANSFERPROCESS_H
