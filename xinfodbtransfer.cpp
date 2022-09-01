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
    g_pPdStruct=nullptr;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB,TT transferType,QString sFileName,XBinary::FT fileType,XBinary::PDSTRUCT *pPdStruct)
{
    g_pXInfoDB=pXInfoDB;
    g_transferType=transferType;
    g_sFileName=sFileName;
    g_fileType=fileType;
    g_pPdStruct=pPdStruct;
}

void XInfoDBTransfer::setData(XInfoDB *pXInfoDB,TT transferType,QIODevice *pDevice,XBinary::FT fileType,XBinary::PDSTRUCT *pPdStruct)
{
    g_pXInfoDB=pXInfoDB;
    g_transferType=transferType;
    g_pDevice=pDevice;
    g_fileType=fileType;
    g_pPdStruct=pPdStruct;
}

bool XInfoDBTransfer::process()
{
    bool bResult=false;

    QElapsedTimer scanTimer;
    scanTimer.start();

    g_pPdStruct->pdRecordOpt.bIsValid=true;

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
                        QList<XELF_DEF::Elf_Phdr> listProgramHeaders=elf.getElf_PhdrList();

                        if(memoryMap.nEntryPointAddress)
                        {
                            g_pXInfoDB->_addSymbol(memoryMap.nEntryPointAddress,0,0,"EntryPoint",XInfoDB::ST_ENTRYPOINT,XInfoDB::SS_FILE);
                        }

                        QList<XELF::TAG_STRUCT> listTagStructs=elf.getTagStructs(&listProgramHeaders,&memoryMap);

                        QList<XELF::TAG_STRUCT> listDynSym=elf._getTagStructs(&listTagStructs,XELF_DEF::DT_SYMTAB);
                        QList<XELF::TAG_STRUCT> listStrTab=elf._getTagStructs(&listTagStructs,XELF_DEF::DT_STRTAB);
                        QList<XELF::TAG_STRUCT> listStrSize=elf._getTagStructs(&listTagStructs,XELF_DEF::DT_STRSZ);

                        if(listDynSym.count()&&listStrTab.count()&&listStrSize.count())
                        {
                            qint64 nSymTabOffset=XBinary::addressToOffset(&memoryMap,listDynSym.at(0).nValue);
                            qint64 nStringTableOffset=XBinary::addressToOffset(&memoryMap,listStrTab.at(0).nValue);
                            qint64 nStringTableSize=listStrSize.at(0).nValue;

                            bool bIs64=elf.is64();
                            bool bIsBigEndian=elf.isBigEndian();

                            if(bIs64)
                            {
                                nSymTabOffset+=sizeof(XELF_DEF::Elf64_Sym);
                            }
                            else
                            {
                                nSymTabOffset+=sizeof(XELF_DEF::Elf32_Sym);
                            }

                            while(!(g_pPdStruct->bIsStop))
                            {
                                XELF_DEF::Elf_Sym record={};

                                if(bIs64)
                                {
                                    XELF_DEF::Elf64_Sym _record=elf._readElf64_Sym(nSymTabOffset,bIsBigEndian);

                                    record.st_name=_record.st_name;
                                    record.st_info=_record.st_info;
                                    record.st_other=_record.st_other;
                                    record.st_shndx=_record.st_shndx;
                                    record.st_value=_record.st_value;
                                    record.st_size=_record.st_size;

                                    nSymTabOffset+=sizeof(XELF_DEF::Elf64_Sym);
                                }
                                else
                                {
                                    XELF_DEF::Elf32_Sym _record=elf._readElf32_Sym(nSymTabOffset,bIsBigEndian);

                                    record.st_name=_record.st_name;
                                    record.st_info=_record.st_info;
                                    record.st_other=_record.st_other;
                                    record.st_shndx=_record.st_shndx;
                                    record.st_value=_record.st_value;
                                    record.st_size=_record.st_size;

                                    nSymTabOffset+=sizeof(XELF_DEF::Elf32_Sym);
                                }

                                if((!record.st_info)||(record.st_other))
                                {
                                    break;
                                }

                                XADDR nSymbolAddress=record.st_value;
                                quint64 nSymbolSize=record.st_size;

                                qint32 nBind=S_ELF64_ST_BIND(record.st_info);
                                qint32 nType=S_ELF64_ST_TYPE(record.st_info);

                                if(nSymbolAddress)
                                {
                                    if((nBind==1)||(nBind==2)) // GLOBAL,WEAK TODO consts
                                    {
                                        if((nType==0)||(nType==1)||(nType==2)) // NOTYPE,OBJECT,FUNC TODO consts
                                        {
                                            XInfoDB::ST symbolType=XInfoDB::ST_LABEL;

                                            if      (nType==0)      symbolType=XInfoDB::ST_LABEL;
                                            else if (nType==1)      symbolType=XInfoDB::ST_OBJECT;
                                            else if (nType==2)      symbolType=XInfoDB::ST_FUNCTION;

                                            QString sSymbolName=elf.getStringFromIndex(nStringTableOffset,nStringTableSize,record.st_name);

                                            if(XBinary::isAddressValid(&memoryMap,nSymbolAddress))
                                            {
                                                g_pXInfoDB->_addSymbol(nSymbolAddress,nSymbolSize,0,sSymbolName,symbolType,XInfoDB::SS_FILE);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
                {
                    XPE pe(pDevice);

                    if(pe.isValid())
                    {
                        XBinary::_MEMORY_MAP memoryMap=pe.getMemoryMap();

                        g_pXInfoDB->_addSymbol(memoryMap.nEntryPointAddress,0,0,"EntryPoint",XInfoDB::ST_ENTRYPOINT,XInfoDB::SS_FILE);

                        {
                            XPE::EXPORT_HEADER _export=pe.getExport(&memoryMap,false,g_pPdStruct);

                            qint32 nNumberOfRecords=_export.listPositions.count();

                            for(qint32 i=0;(i<nNumberOfRecords)&&(!(g_pPdStruct->bIsStop));i++)
                            {
                                QString sFunctionName=_export.listPositions.at(i).sFunctionName;

                                if(sFunctionName=="")
                                {
                                    sFunctionName=QString::number(_export.listPositions.at(i).nOrdinal);
                                }

                                g_pXInfoDB->_addSymbol(_export.listPositions.at(i).nAddress,0,0,sFunctionName,XInfoDB::ST_EXPORT,XInfoDB::SS_FILE);
                            }
                        }
                        {
                            QList<XPE::IMPORT_RECORD> listImportRecords=pe.getImportRecords(&memoryMap,g_pPdStruct);

                            qint32 nNumberOfRecords=listImportRecords.count();

                            for(qint32 i=0;(i<nNumberOfRecords)&&(!(g_pPdStruct->bIsStop));i++)
                            {
                                QString sFunctionName=listImportRecords.at(i).sLibrary+"#"+listImportRecords.at(i).sFunction;

                                g_pXInfoDB->_addSymbol(XBinary::relAddressToAddress(&memoryMap,listImportRecords.at(i).nRVA),0,0,sFunctionName,XInfoDB::ST_IMPORT,XInfoDB::SS_FILE);
                            }
                        }
                        // TODO TLS
                        // TODO More
                    }
                }
            }

            g_pXInfoDB->_sortSymbols();

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

    if(!(g_pPdStruct->bIsStop))
    {
        g_pPdStruct->pdRecordOpt.bSuccess=true;
    }

    g_pPdStruct->pdRecordOpt.bFinished=true;

    emit completed(scanTimer.elapsed());

    return bResult;
}
