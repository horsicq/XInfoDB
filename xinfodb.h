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
#ifndef XINFODB_H
#define XINFODB_H

#include <QObject>
#include "xbinary.h"
#include "xprocess.h" // TODO def

class XInfoDB : public QObject
{
    Q_OBJECT
public:

    enum MODE
    {
        MODE_UNKNOWN=0,
        MODE_PROCESS
    };

    struct XREG_OPTIONS
    {
        bool bGeneral;
        bool bIP;
        bool bFlags;
        bool bSegments;
        bool bDebug;
        bool bFloat;
        bool bXMM;
    };

    enum XREG
    {
        XREG_UNKNOWN=0,
        XREG_EAX,
        XREG_ECX,
        XREG_EDX,
        XREG_EBX,
        XREG_ESP,
        XREG_EBP,
        XREG_ESI,
        XREG_EDI,
        XREG_EIP,
        XREG_RAX,
        XREG_RCX,
        XREG_RDX,
        XREG_RBX,
        XREG_RSP,
        XREG_RBP,
        XREG_RSI,
        XREG_RDI,
        XREG_R8,
        XREG_R9,
        XREG_R10,
        XREG_R11,
        XREG_R12,
        XREG_R13,
        XREG_R14,
        XREG_R15,
        XREG_RIP,
        XREG_EFLAGS,
        XREG_CS,
        XREG_DS,
        XREG_ES,
        XREG_FS,
        XREG_GS,
        XREG_SS,
        XREG_DR0,
        XREG_DR1,
        XREG_DR2,
        XREG_DR3,
        XREG_DR6,
        XREG_DR7,
        XREG_CF,
        XREG_PF,
        XREG_AF,
        XREG_ZF,
        XREG_SF,
        XREG_TF,
        XREG_IF,
        XREG_DF,
        XREG_OF,
        XREG_ST0,
        XREG_ST1,
        XREG_ST2,
        XREG_ST3,
        XREG_ST4,
        XREG_ST5,
        XREG_ST6,
        XREG_ST7,
        XREG_XMM0,
        XREG_XMM1,
        XREG_XMM2,
        XREG_XMM3,
        XREG_XMM4,
        XREG_XMM5,
        XREG_XMM6,
        XREG_XMM7,
        XREG_XMM8,
        XREG_XMM9,
        XREG_XMM10,
        XREG_XMM11,
        XREG_XMM12,
        XREG_XMM13,
        XREG_XMM14,
        XREG_XMM15,
    };

    explicit XInfoDB(QObject *pParent=nullptr);
    ~XInfoDB();

    void setProcess(XProcess::HANDLEID hidProcess);
    void updateRegs(XProcess::HANDLEID hidThread,XREG_OPTIONS regOptions);
    void updateMemoryRegionsList();
    void updateModulesList();
    XBinary::XVARIANT getCurrentReg(XREG reg);
    QList<XBinary::MEMORY_REGION> *getCurrentMemoryRegionsList();
    QList<XBinary::MODULE> *getCurrentModulesList();
    bool isRegChanged(XREG reg);

    static QString regIdToString(XREG reg);

private:

    struct STATUS
    {
        QMap<XREG,XBinary::XVARIANT> mapRegs;
        QList<XBinary::MEMORY_REGION> listMemoryRegions;
        QList<XBinary::MODULE> listModules;
    };

    XBinary::XVARIANT _getReg(QMap<XREG,XBinary::XVARIANT> *pMapRegs,XREG reg);

private:
    XProcess::HANDLEID g_hidProcess;
    MODE g_mode;
    STATUS g_statusCurrent;
    STATUS g_statusPrev;
};

#endif // XINFODB_H
