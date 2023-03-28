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
    g_pActionAnalyze = nullptr;
    g_pActionLoad = nullptr;
    g_pActionSave = nullptr;
    g_pActionClear = nullptr;
    g_pXInfoDB = nullptr;
    g_pDevice = nullptr;
}

QMenu *XInfoMenu::createMenu(QWidget *pParent)
{
    g_pParent = pParent;

    g_pMenu = new QMenu(tr("Database"), pParent);

    g_pActionAnalyze = new QAction(tr("Analyze"), pParent);
    g_pActionLoad = new QAction(tr("Load"), pParent);
    g_pActionSave = new QAction(tr("Save"), pParent);
    g_pActionClear = new QAction(tr("Clear"), pParent);

    g_pMenu->addAction(g_pActionAnalyze);
    g_pMenu->addAction(g_pActionLoad);
    g_pMenu->addAction(g_pActionSave);
    g_pMenu->addSeparator();
    g_pMenu->addAction(g_pActionClear);

    connect(g_pActionAnalyze, SIGNAL(triggered()), this, SLOT(actionAnalyze()));
    connect(g_pActionLoad, SIGNAL(triggered()), this, SLOT(actionLoad()));
    connect(g_pActionSave, SIGNAL(triggered()), this, SLOT(actionSave()));
    connect(g_pActionClear, SIGNAL(triggered()), this, SLOT(actionClear()));

    updateMenu();

    return g_pMenu;
}

void XInfoMenu::setData(XInfoDB *pXInfoDB, QIODevice *pDevice)
{
    g_pXInfoDB = pXInfoDB;
    g_pDevice = pDevice;

    if (g_pXInfoDB) {
//        connect(g_pXInfoDB, SIGNAL(analyzeStateChanged()), this, SLOT(updateMenu()));
    }

    updateMenu();
}

void XInfoMenu::clear()
{
    setData(nullptr, nullptr);
}

void XInfoMenu::updateMenu()
{
#ifdef QT_MENU
    qDebug("void XInfoMenu::updateMenu()");
#endif
}

void XInfoMenu::actionAnalyze()
{
#ifdef QT_MENU
    qDebug("void XInfoMenu::actionAnalyze()");
#endif
}

void XInfoMenu::actionLoad()
{

}

void XInfoMenu::actionSave()
{

}

void XInfoMenu::actionClear()
{

}
