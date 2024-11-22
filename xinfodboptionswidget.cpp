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
#include "xinfodboptionswidget.h"

#include "ui_xinfodboptionswidget.h"

XInfoDBOptionsWidget::XInfoDBOptionsWidget(QWidget *pParent) : XShortcutsWidget(pParent), ui(new Ui::XInfoDBOptionsWidget)
{
    ui->setupUi(this);

    g_pOptions = nullptr;

    setProperty("GROUPID", XOptions::GROUPID_INFO);
}

XInfoDBOptionsWidget::~XInfoDBOptionsWidget()
{
    delete ui;
}

void XInfoDBOptionsWidget::adjustView()
{
    // TODO
}

void XInfoDBOptionsWidget::setOptions(XOptions *pOptions)
{
    g_pOptions = pOptions;

    reload();
}

void XInfoDBOptionsWidget::save()
{
    g_pOptions->getLineEdit(ui->lineEditInfoPath, XOptions::ID_INFO_PATH);
}

void XInfoDBOptionsWidget::setDefaultValues(XOptions *pOptions)
{
    pOptions->addID(XOptions::ID_INFO_PATH, "$data/info");
}

void XInfoDBOptionsWidget::reloadData(bool bSaveSelection)
{
    Q_UNUSED(bSaveSelection)
    reload();
}

void XInfoDBOptionsWidget::reload()
{
    g_pOptions->setLineEdit(ui->lineEditInfoPath, XOptions::ID_INFO_PATH);
}

void XInfoDBOptionsWidget::on_toolButtonInfoPath_clicked()
{
    QString sText = ui->lineEditInfoPath->text();
    QString sInitDirectory = XBinary::convertPathName(sText);

    QString sDirectoryName = QFileDialog::getExistingDirectory(this, tr("Open directory") + QString("..."), sInitDirectory, QFileDialog::ShowDirsOnly);

    if (!sDirectoryName.isEmpty()) {
        ui->lineEditInfoPath->setText(sDirectoryName);
    }
}

void XInfoDBOptionsWidget::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
}
