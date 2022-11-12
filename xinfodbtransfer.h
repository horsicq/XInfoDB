/* Copyright (c) 2022 hors<horsicq@gmail.com>
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
#ifndef XINFODBTRANSFER_H
#define XINFODBTRANSFER_H

#include "xformats.h"
#include "xinfodb.h"

class XInfoDBTransfer : public QObject {
    Q_OBJECT
public:
    explicit XInfoDBTransfer(QObject *pParent = nullptr);

    enum TT {
        TT_IMPORT = 0,
        TT_EXPORT
    };

    void setData(XInfoDB *pXInfoDB, TT transferType, QString sFileName, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct);
    void setData(XInfoDB *pXInfoDB, TT transferType, QIODevice *pDevice, XBinary::FT fileType, XBinary::PDSTRUCT *pPdStruct);
    //    bool loadFromFile(QString sFileName,XBinary::FT fileType);

public slots:
    bool process();

signals:
    void errorMessage(QString sText);
    void completed(qint64 nElapsed);

private:
    XInfoDB *g_pXInfoDB;
    TT g_transferType;
    QString g_sFileName;
    QIODevice *g_pDevice;
    XBinary::FT g_fileType;
    XBinary::PDSTRUCT *g_pPdStruct;
};

#endif  // XINFODBTRANSFER_H
