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
#include "xinfodbtransfer.h"

XInfoDBTransfer::XInfoDBTransfer(QObject *pParent)
    : QObject(pParent)
{
    g_pXInfoDB=nullptr;
    g_transferType=TT_IMPORT;
    g_fileType=XBinary::FT_UNKNOWN;
    g_pDevice=nullptr;
    g_bIsStop=false;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB,TT transferType,QString sFileName,XBinary::FT fileType)
{
    g_pXInfoDB=pXInfoDB;
    g_transferType=transferType;
    g_sFileName=sFileName;
    g_fileType=fileType;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB, TT transferType, QIODevice *pDevice, XBinary::FT fileType)
{
    g_pXInfoDB=pXInfoDB;
    g_transferType=transferType;
    g_pDevice=pDevice;
    g_fileType=fileType;
}

bool XInfoDBTransfer::process()
{
    bool bResult=false;
    g_bIsStop=false;

    QElapsedTimer scanTimer;
    scanTimer.start();

    if(g_pXInfoDB)
    {
        if(g_transferType==TT_IMPORT)
        {
            QIODevice *pDevice=g_pDevice;

            bool bFile=false;

            if((!pDevice)&&(g_sFileName!=""))
            {
                bFile=true;

                QFile *pFile=new QFile;

                pFile->setFileName(g_sFileName);

                if(pFile->open(QIODevice::ReadOnly))
                {
                    pDevice=pFile;
                }
                else
                {
                    delete pFile;
                }
            }

            if(pDevice)
            {
                XBinary::FT fileType=g_fileType;

                if(fileType==XBinary::FT_UNKNOWN)
                {
                    fileType=XBinary::getPrefFileType(pDevice);
                }

                if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
                {
                    XELF elf(pDevice);

                    if(elf.isValid())
                    {
                        XBinary::_MEMORY_MAP memoryMap=elf.getMemoryMap();

                        g_pXInfoDB->_addSymbol(memoryMap.nEntryPointAddress,0,"EntryPoint",XInfoDB::ST_ENTRYPOINT,XInfoDB::SS_FILE);
                        g_pXInfoDB->_sortSymbols();
                    }
                }
            }

            if(bFile&&pDevice)
            {
                QFile *pFile=static_cast<QFile *>(pDevice);

                pFile->close();

                delete pFile;
            }
        }
        else if(g_transferType==TT_EXPORT)
        {
            // TODO
        }
    }

    emit completed(scanTimer.elapsed());

    return bResult;
}

void XInfoDBTransfer::stop()
{
    g_bIsStop=true;
}
