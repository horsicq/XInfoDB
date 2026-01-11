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
#include "xinfomenu.h"

XInfoMenu::XInfoMenu(XShortcuts *pShortcuts, XOptions *pXOptions)
{
    m_pShortcuts = pShortcuts;
    m_pXOptions = pXOptions;
    m_pParent = nullptr;
    m_pMenu = nullptr;
    //    m_pActionAnalyze = nullptr;
    m_pActionExport = nullptr;
    m_pActionImport = nullptr;
    m_pXInfoDB = nullptr;
    m_pDevice = nullptr;
}

QMenu *XInfoMenu::createMenu(QWidget *pParent)
{
    m_pParent = pParent;

    m_pMenu = new QMenu(tr("Database"), pParent);

    //    m_pActionAnalyze = new QAction(tr("Analyze"), pParent);
    m_pActionImport = new QAction(tr("Import"), pParent);
    m_pActionExport = new QAction(tr("Export"), pParent);

    //    m_pMenu->addAction(m_pActionAnalyze);
    m_pMenu->addAction(m_pActionExport);
    m_pMenu->addAction(m_pActionImport);

    //    connect(m_pActionAnalyze, SIGNAL(triggered()), this, SLOT(actionAnalyze()));
    connect(m_pActionExport, SIGNAL(triggered()), this, SLOT(actionExport()));
    connect(m_pActionImport, SIGNAL(triggered()), this, SLOT(actionImport()));

    updateMenu();

    return m_pMenu;
}

void XInfoMenu::setData(XInfoDB *pXInfoDB)
{
    m_pXInfoDB = pXInfoDB;

    if (pXInfoDB) {
        connect(pXInfoDB, SIGNAL(reloadViewSignal()), this, SLOT(updateMenu()));
        pXInfoDB->reloadView();
    }
}

void XInfoMenu::setData(XInfoDB *pXInfoDB, QIODevice *pDevice, const QString &sDatabaseFileName)
{
    setData(pXInfoDB);
    m_pDevice = pDevice;
    m_sDatabaseFileName = sDatabaseFileName;
}

void XInfoMenu::tryToSave()
{
    if (m_pXInfoDB->isDatabaseChanged()) {
        QString _sFileName = getDatabaseFileName();
        QString _sString = QString("%1 \"%2\"?").arg(tr("Save"), _sFileName);

        if (QMessageBox::question(m_pParent, tr("Database"), _sString) == QMessageBox::Yes) {
            save(_sFileName);
        }
    }
}

void XInfoMenu::tryToLoad()
{
    QString _sFileName = getDatabaseFileName();

    if (XBinary::isFileExists(_sFileName)) {
        QString _sString = QString("%1 \"%2\"?").arg(tr("Load"), _sFileName);

        if (QMessageBox::question(m_pParent, tr("Database"), _sString) == QMessageBox::Yes) {
            load(_sFileName);
        }
    }
}

void XInfoMenu::reset()
{
    setData(nullptr);
}

QString XInfoMenu::getDatabaseFileName()
{
    return m_sDatabaseFileName;
}

void XInfoMenu::updateMenu()
{
    if (m_pXInfoDB) {
        bool bIsDatabasePresent = m_pXInfoDB->isDbPresent();

        m_pActionExport->setEnabled(bIsDatabasePresent);
        m_pActionImport->setEnabled(true);
    } else {
        m_pActionExport->setEnabled(false);
        m_pActionImport->setEnabled(false);
    }
}

// void XInfoMenu::actionAnalyze()
//{
// #ifdef QT_MENU
//     qDebug("void XInfoMenu::actionAnalyze()");
// #endif
// }

void XInfoMenu::actionExport()
{
    if (m_pXInfoDB) {
        QString _sFileName = getDatabaseFileName();
        _sFileName = QFileDialog::getSaveFileName(m_pParent, tr("Save"), _sFileName, QString("%1 (*.db);;%2 (*)").arg(tr("Database"), tr("All files")));

        if (!_sFileName.isEmpty()) {
            load(_sFileName);
        }
    }
}

void XInfoMenu::actionImport()
{
    if (m_pXInfoDB) {
        QString _sFileName;
        //= XBinary::getDeviceDirectory(m_pXInfoDB->getDevice());
        _sFileName = QFileDialog::getOpenFileName(m_pParent, tr("Open file") + QString("..."), _sFileName, tr("Database") + QString(" (*.db)"));

        if (!_sFileName.isEmpty()) {
            save(_sFileName);
        }
    }
}

void XInfoMenu::save(const QString &sFileName)
{
    XInfoDBTransfer::OPTIONS options = {};
    options.sDatabaseFileName = sFileName;
    // options.nModuleAddress = -1;

    XInfoDBTransfer infoTransfer;
    XDialogProcess dialogTransfer(m_pParent, &infoTransfer);
    dialogTransfer.setGlobal(m_pShortcuts, m_pXOptions);
    infoTransfer.setData(m_pXInfoDB, XInfoDBTransfer::COMMAND_IMPORT, options, dialogTransfer.getPdStruct());
    dialogTransfer.start();
    dialogTransfer.showDialogDelay();
    m_pXInfoDB->reloadView();
}

void XInfoMenu::load(const QString &sFileName)
{
    XInfoDBTransfer::OPTIONS options = {};
    options.sDatabaseFileName = sFileName;
    options.pDevice = m_pDevice;
    // options.nModuleAddress = -1;

    XInfoDBTransfer infoTransfer;
    XDialogProcess dialogTransfer(m_pParent, &infoTransfer);
    dialogTransfer.setGlobal(m_pShortcuts, m_pXOptions);
    infoTransfer.setData(m_pXInfoDB, XInfoDBTransfer::COMMAND_EXPORT, options, dialogTransfer.getPdStruct());
    dialogTransfer.start();
    dialogTransfer.showDialogDelay();
    m_pXInfoDB->reloadView();
}
