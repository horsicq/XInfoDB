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
#include "xinfomenu.h"

XInfoMenu::XInfoMenu()
{
    g_pParent = nullptr;
    g_pMenu = nullptr;
    //    g_pActionAnalyze = nullptr;
    g_pActionExport = nullptr;
    g_pActionImport = nullptr;
    g_pActionClear = nullptr;
    g_pXInfoDB = nullptr;
}

QMenu *XInfoMenu::createMenu(QWidget *pParent)
{
    g_pParent = pParent;

    g_pMenu = new QMenu(tr("Database"), pParent);

    //    g_pActionAnalyze = new QAction(tr("Analyze"), pParent);
    g_pActionImport = new QAction(tr("Import"), pParent);
    g_pActionExport = new QAction(tr("Export"), pParent);
    g_pActionClear = new QAction(tr("Clear"), pParent);

    //    g_pMenu->addAction(g_pActionAnalyze);
    g_pMenu->addAction(g_pActionExport);
    g_pMenu->addAction(g_pActionImport);
    g_pMenu->addSeparator();
    g_pMenu->addAction(g_pActionClear);

    //    connect(g_pActionAnalyze, SIGNAL(triggered()), this, SLOT(actionAnalyze()));
    connect(g_pActionExport, SIGNAL(triggered()), this, SLOT(actionExport()));
    connect(g_pActionImport, SIGNAL(triggered()), this, SLOT(actionImport()));
    connect(g_pActionClear, SIGNAL(triggered()), this, SLOT(actionClear()));

    updateMenu();

    return g_pMenu;
}

void XInfoMenu::setData(XInfoDB *pXInfoDB)
{
    g_pXInfoDB = pXInfoDB;

    updateMenu();
}

void XInfoMenu::reset()
{
    setData(nullptr);
}

void XInfoMenu::updateMenu()
{
    if (g_pXInfoDB) {
        bool bIsDatabasePresent = false;

        bIsDatabasePresent = g_pXInfoDB->isDbPresent();

        g_pActionExport->setEnabled(bIsDatabasePresent);
        g_pActionImport->setEnabled(!bIsDatabasePresent);
        g_pActionClear->setEnabled(bIsDatabasePresent);
        //        connect(g_pXInfoDB, SIGNAL(analyzeStateChanged()), this, SLOT(updateMenu()));
    } else {
        g_pActionExport->setEnabled(false);
        g_pActionImport->setEnabled(false);
        g_pActionClear->setEnabled(false);
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
    if (g_pXInfoDB) {
        QString _sFileName = XBinary::getDeviceDirectory(g_pXInfoDB->getDevice()) + QDir::separator() + XBinary::getDeviceFileName(g_pXInfoDB->getDevice()) + ".db";
        _sFileName = QFileDialog::getSaveFileName(g_pParent, tr("Save"), _sFileName, QString("%1 (*.db);;%2 (*)").arg(tr("Database"), tr("All files")));

        if (!_sFileName.isEmpty()) {
            // TODO
        }
    }
}

void XInfoMenu::actionImport()
{
    if (g_pXInfoDB) {
        QString _sFileName = XBinary::getDeviceDirectory(g_pXInfoDB->getDevice());
        _sFileName = QFileDialog::getOpenFileName(g_pParent, tr("Open file") + QString("..."), _sFileName, tr("Database") + QString(" (*.db)"));

        if (!_sFileName.isEmpty()) {
            // TODO
        }
    }
}

void XInfoMenu::actionClear()
{
    if (g_pXInfoDB) {
        if (QMessageBox::question(g_pParent, tr("Database"), tr("Are you sure?")) == QMessageBox::Yes) {
            g_pXInfoDB->clearDb();
            updateMenu();
        }
    }
}
