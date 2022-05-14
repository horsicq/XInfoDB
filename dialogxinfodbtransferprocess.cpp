/* Copyright (c) 2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
#include "ui_dialogxinfodbtransferprocess.h"

DialogXInfoDBTransferProcess::DialogXInfoDBTransferProcess(QWidget *pParent) :
    QDialog(pParent),
    ui(new Ui::DialogXInfoDBTransferProcess)
{
    ui->setupUi(this);

    g_bIsStop=false;

    g_pTransfer=new XInfoDBTransfer;
    g_pThread=new QThread;

    g_pTransfer->moveToThread(g_pThread);

    connect(g_pThread,SIGNAL(started()),g_pTransfer,SLOT(process()));
    connect(g_pTransfer,SIGNAL(completed(qint64)),this,SLOT(onCompleted(qint64)));
    connect(g_pTransfer,SIGNAL(errorMessage(QString)),this,SLOT(errorMessage(QString)));
//    connect(g_pTransfer,SIGNAL(progressValueChanged(qint32)),this,SLOT(onProgressValueChanged(qint32)));

//    ui->progressBar->setMaximum(100);
//    ui->progressBar->setMinimum(0);
}

DialogXInfoDBTransferProcess::~DialogXInfoDBTransferProcess()
{
//    g_pTransfer->stop();

    g_pThread->quit();
    g_pThread->wait();

    delete ui;

    delete g_pThread;
    delete g_pTransfer;
}

void DialogXInfoDBTransferProcess::importData(XInfoDB *pXInfoDB,QString sFileName,XBinary::FT fileType)
{
    setWindowTitle(tr("Import"));

    g_pTransfer->setData(pXInfoDB,XInfoDBTransfer::TT_IMPORT,sFileName,fileType);
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::importData(XInfoDB *pXInfoDB,QIODevice *pDevice,XBinary::FT fileType)
{
    setWindowTitle(tr("Import"));

    g_pTransfer->setData(pXInfoDB,XInfoDBTransfer::TT_IMPORT,pDevice,fileType);
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::exportData(XInfoDB *pXInfoDB,QString sFileName)
{
    setWindowTitle(tr("Export"));

    g_pTransfer->setData(pXInfoDB,XInfoDBTransfer::TT_EXPORT,sFileName,XBinary::FT_UNKNOWN);
    g_pThread->start();
}

void DialogXInfoDBTransferProcess::on_pushButtonCancel_clicked()
{
    g_bIsStop=true;

    g_pTransfer->stop();
}

void DialogXInfoDBTransferProcess::errorMessage(QString sText)
{

}

void DialogXInfoDBTransferProcess::onCompleted(qint64 nElapsed)
{
    Q_UNUSED(nElapsed)

    if(!g_bIsStop)
    {
        accept();
    }
    else
    {
        reject();
    }
}

