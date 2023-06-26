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
#ifndef XINFOMENU_H
#define XINFOMENU_H

#include <QObject>
#include <QMenu>
#include "dialogxinfodbtransferprocess.h"

class XInfoMenu : public QObject {
    Q_OBJECT
public:
    explicit XInfoMenu();
    QMenu *createMenu(QWidget *pParent);
    void setData(XInfoDB *pXInfoDB);
    void reset();

private slots:
    void updateMenu();
    //    void actionAnalyze();
    void actionExport();
    void actionImport();
    void actionClear();

private:
    QWidget *g_pParent;
    QMenu *g_pMenu;
    //    QAction *g_pActionAnalyze;
    QAction *g_pActionExport;
    QAction *g_pActionImport;
    QAction *g_pActionClear;
    XInfoDB *g_pXInfoDB;
};

#endif  // XINFOMENU_H
